#pragma once
#include <string>
#include <vector>
#include "injectorUI.h"
#include "common/remoteprocess.h"

#define IMR_RELTYPE(x)				((x >> 12) & 0xF)
#define IMR_RELOFFSET(x)			(x & 0xFFF)

static_assert(sizeof(uintptr_t) == 8, "64bit");

class Injector {
private:
	const RemoteProcess& process;
	mutable std::string strMsg;
	std::vector<uint8_t> Binary;

	template<class Type>
	void GetBinaryData(uintptr_t Offset, size_t Index, Type& Data) const {
		std::copy_n(Binary.begin() + Offset + sizeof(Type) * Index, sizeof(Type), (uint8_t*)&Data);
	}

	template<class Type>
	void SetBinaryData(uintptr_t Offset, size_t Index, const Type& Data) {
		std::copy_n((const uint8_t*)&Data, sizeof(Type), Binary.begin() + Offset + sizeof(Type) * Index);
	}

	HMODULE RemoteGetModuleHandleA(const char* szModuleName) const {
		HMODULE Result = 0;
		bool bSuccess = false;

		if (szModuleName)
			process.VirtualAllocWrapper(szModuleName, strlen(szModuleName) + 1, [&](const void* pszModuleName) {
			bSuccess = process.RemoteCall(GetModuleHandleA, &Result, pszModuleName, 0, 0, 0, true, false); });
		else
			bSuccess = process.RemoteCall(GetModuleHandleA, &Result, 0, 0, 0, 0, true, false);

		return bSuccess ? Result : 0;
	}

	HMODULE RemoteLoadLibraryExA(const char* szFileName, DWORD dwFlags = 0) const {
		HMODULE Result = 0;
		bool bSuccess = false;

		if (szFileName)
			process.VirtualAllocWrapper(szFileName, strlen(szFileName) + 1, [&](const void* pszFileName) {
			bSuccess = process.RemoteCall(LoadLibraryExA, &Result, pszFileName, 0, dwFlags, 0, true, false); });
		else
			bSuccess = process.RemoteCall(LoadLibraryExA, &Result, 0, 0, dwFlags, 0, true, false);

		return bSuccess ? Result : 0;
	}

	FARPROC RemoteGetProcAddress(HMODULE hModule, const char* szFuncName) const {
		FARPROC Result = 0;
		bool bSuccess = false;

		if (HIWORD(szFuncName))
			process.VirtualAllocWrapper(szFuncName, strlen(szFuncName) + 1, [&](const void* pszFuncName) {
			bSuccess = process.RemoteCall(GetProcAddress, &Result, hModule, pszFuncName, 0, 0, true, false); });
		else
			bSuccess = process.RemoteCall(GetProcAddress, &Result, hModule, szFuncName, 0, 0, true, false);

		return bSuccess ? Result : 0;
	}

	size_t GetOffsetOptionalHeader(size_t OffsetNtHeader, const IMAGE_NT_HEADERS& ntHd) const {
		return OffsetNtHeader + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + ntHd.FileHeader.SizeOfOptionalHeader;
	}

	bool GetSectionHeaderInRVA(size_t OffsetNtHeader, const IMAGE_NT_HEADERS& ntHd, uintptr_t RVA, IMAGE_SECTION_HEADER& OutSectionHeader) const {
		size_t OffsetOptionalHeader = GetOffsetOptionalHeader(OffsetNtHeader, ntHd);
		if (!OffsetOptionalHeader)
			return false;

		for (unsigned i = 0; i < ntHd.FileHeader.NumberOfSections; i++) {
			IMAGE_SECTION_HEADER SectionHeader;
			GetBinaryData(OffsetOptionalHeader, i, SectionHeader);

			size_t SectionSize = SectionHeader.Misc.VirtualSize;
			if (!SectionSize)
				SectionSize = SectionHeader.SizeOfRawData;

			if (RVA >= SectionHeader.VirtualAddress && RVA < (SectionHeader.VirtualAddress + SectionSize)) {
				OutSectionHeader = SectionHeader;
				return true;
			}
		}
		return false;
	}

	uintptr_t GetOffsetFromRVA(size_t OffsetNtHeader, const IMAGE_NT_HEADERS& ntHd, uintptr_t RVA) const {
		IMAGE_SECTION_HEADER SectionHeader;
		if (!GetSectionHeaderInRVA(OffsetNtHeader, ntHd, RVA, SectionHeader))
			return 0;

		return RVA - SectionHeader.VirtualAddress + SectionHeader.PointerToRawData;
	}

	bool CheckHeader(size_t& OffsetNtHeader, IMAGE_NT_HEADERS& ntHd) const {
		IMAGE_DOS_HEADER dosHd;
		GetBinaryData(0, 0, dosHd);
		if (dosHd.e_magic != IMAGE_DOS_SIGNATURE) {
			strMsg = (std::string)"Invalid DOS signature"e;
			return false;
		}

		OffsetNtHeader = dosHd.e_lfanew;
		GetBinaryData(OffsetNtHeader, 0, ntHd);
		if (ntHd.Signature != IMAGE_NT_SIGNATURE) {
			strMsg = (std::string)"Invalid NT signature"e;
			return false;
		}

		if (!(ntHd.FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)) {
			strMsg = (std::string)"File is not executable"e;
			return false;
		}

		if (ntHd.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
			strMsg = (std::string)"File is not 64bit application"e;
			return false;
		}

		if (!process.IsPlatformMatched()) {
			strMsg = (std::string)"Target process is not 64bit process"e;
			return false;
		}

		return true;
	}

	bool FixImports(size_t OffsetNtHeader, const IMAGE_NT_HEADERS& ntHd, const char* szDLLDir) {
		if (!ntHd.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
			return true;
		
		const size_t OffsetImportDesc = GetOffsetFromRVA(OffsetNtHeader, ntHd,
			ntHd.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

		char szSystemDirectory[MAX_PATH];
		if (!GetSystemDirectoryA(szSystemDirectory, sizeof(szSystemDirectory))) {
			strMsg = (std::string)"GetSystemDirectoryA Failed"e;
			return false;
		}

		for (unsigned ImportDescIndex = 0;; ImportDescIndex++) {
			IMAGE_IMPORT_DESCRIPTOR ImportDesc;
			GetBinaryData(OffsetImportDesc, ImportDescIndex, ImportDesc);

			const uintptr_t NameOffset = GetOffsetFromRVA(OffsetNtHeader, ntHd, ImportDesc.Name);
			if (!NameOffset)
				break;

			const std::string strDLLName = (const char*)&Binary[NameOffset];
			if (!strDLLName.size())
				break;

			HMODULE hRemoteDLL = RemoteLoadLibraryExA(strDLLName.c_str());
			if (!hRemoteDLL) {
				//System32 folder
				std::string strSystemPathDLL = szSystemDirectory;
				strSystemPathDLL += (const char*)"\\"e;
				strSystemPathDLL += strDLLName;
				hRemoteDLL = RemoteLoadLibraryExA(strSystemPathDLL.c_str());
			}
			if (!hRemoteDLL) {
				//Injector folder
				std::string strPathDLL = szDLLDir;
				strPathDLL += (const char*)"\\"e;
				strPathDLL += strDLLName;
				hRemoteDLL = RemoteLoadLibraryExA(strPathDLL.c_str());
			}

			char szFileName[MAX_PATH];
			if (!hRemoteDLL || !process.RemoteGetModuleFileNameExA(hRemoteDLL, szFileName, sizeof(szFileName))) {
				strMsg = (std::string)"RemoteLoadLibrary failed : "e;
				strMsg += strDLLName;
				return false;
			}

			const HMODULE hLocalDLL = LoadLibraryA(szFileName);
			if (!hLocalDLL) {
				strMsg = (std::string)"LocalLibrary failed : "e;
				strMsg += strDLLName;
				return false;
			}

			MODULEINFO LocalDLLInfo, RemoteDLLInfo;
			if (!process.RemoteGetModuleInformation(hRemoteDLL, &RemoteDLLInfo, sizeof(RemoteDLLInfo)) ||
				!GetModuleInformation((HANDLE)-1, hLocalDLL, &LocalDLLInfo, sizeof(LocalDLLInfo))) {
				strMsg = (std::string)"GetModuleInformation failed : "e;
				strMsg += strDLLName;
				return false;
			}

			const bool IsDLLDifferent = (LocalDLLInfo.SizeOfImage != RemoteDLLInfo.SizeOfImage);

			//fix the time/date stamp
			//ImportDesc.TimeDateStamp = ntHd.FileHeader.TimeDateStamp;

			const size_t OffsetImageThunkData = GetOffsetFromRVA(OffsetNtHeader, ntHd, ImportDesc.FirstThunk);

			for (unsigned ImageThunkDataIndex = 0;; ImageThunkDataIndex++) {
				IMAGE_THUNK_DATA ImageThunkData;
				GetBinaryData(OffsetImageThunkData, ImageThunkDataIndex, ImageThunkData);

				if (!ImageThunkData.u1.Ordinal)
					break;

				const char* szFuncName = [&] {
					if (IMAGE_SNAP_BY_ORDINAL(ImageThunkData.u1.Ordinal))
						return (const char*)IMAGE_ORDINAL(ImageThunkData.u1.Ordinal);

					size_t OffsetImageImportByName = GetOffsetFromRVA(OffsetNtHeader, ntHd, ImageThunkData.u1.AddressOfData);
					OffsetImageImportByName += offsetof(IMAGE_IMPORT_BY_NAME, Name);
					for (unsigned i = 0; Binary[OffsetImageImportByName + i]; i++);
					return (const char*)&Binary[OffsetImageImportByName];
				}();

				const FARPROC pRemoteFuncAddress = [&]()->FARPROC {
					if (IsDLLDifferent)
						return RemoteGetProcAddress(hRemoteDLL, szFuncName);
					const FARPROC pFuncAddress = GetProcAddress(hLocalDLL, szFuncName);
					if (!pFuncAddress)
						return 0;
					return FARPROC((uintptr_t)pFuncAddress - (uintptr_t)hLocalDLL + (uintptr_t)hRemoteDLL);
				}();

				if (!pRemoteFuncAddress) {
					strMsg = (std::string)"RemoteGetProcAddress failed : "e;
					strMsg += strDLLName;

					if (HIWORD(szFuncName)) {
						strMsg += (const char*)" "e;
						strMsg += szFuncName;
					}
					else {
						strMsg += (const char*)" ordinal "e;
						strMsg += std::to_string(LOWORD(szFuncName));
					}
					return false;
				}

				ImageThunkData.u1.Function = (uintptr_t)pRemoteFuncAddress;
				SetBinaryData(OffsetImageThunkData, ImageThunkDataIndex, ImageThunkData);
			}
		}

		return true;
	}

	bool FixRelocs(size_t OffsetNtHeader, const IMAGE_NT_HEADERS& ntHd, uintptr_t RemoteImageBase) {
		if (!ntHd.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)
			return true;
		
		size_t OffsetBaseRelocation = GetOffsetFromRVA(OffsetNtHeader, ntHd,
			ntHd.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
		const size_t BaseRelocSectionSize = ntHd.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
		const size_t Delta = RemoteImageBase - ntHd.OptionalHeader.ImageBase;

		for (size_t nBytes = 0; nBytes < BaseRelocSectionSize; ) {
			IMAGE_BASE_RELOCATION BaseRelocation;
			GetBinaryData(OffsetBaseRelocation, 0, BaseRelocation);

			const size_t OffsetVA = GetOffsetFromRVA(OffsetNtHeader, ntHd, BaseRelocation.VirtualAddress);
			const size_t NumRelocs = (BaseRelocation.SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(uint16_t);

			size_t OffsetRelocationData = OffsetBaseRelocation + sizeof(IMAGE_BASE_RELOCATION);
			for (unsigned i = 0; i < NumRelocs; i++) {
				uint16_t RelocationData;
				GetBinaryData(OffsetRelocationData, i, RelocationData);

				const uint8_t RelocType = IMR_RELTYPE(RelocationData);
				const size_t OffsetRelocData = OffsetVA + IMR_RELOFFSET(RelocationData);

				switch (RelocType) {
				case IMAGE_REL_BASED_DIR64: {
					uint64_t RelocData;
					GetBinaryData(OffsetRelocData, 0, RelocData);
					RelocData += Delta;
					SetBinaryData(OffsetRelocData, 0, RelocData);
					break;
				}
				case IMAGE_REL_BASED_HIGHLOW: {
					uint32_t RelocData;
					GetBinaryData(OffsetRelocData, 0, RelocData);
					RelocData += uint32_t(Delta);
					SetBinaryData(OffsetRelocData, 0, RelocData);
					break;
				}
				case IMAGE_REL_BASED_HIGH: {
					uint16_t RelocData;
					GetBinaryData(OffsetRelocData, 0, RelocData);
					RelocData += HIWORD(Delta);
					SetBinaryData(OffsetRelocData, 0, RelocData);
					break;
				}
				case IMAGE_REL_BASED_LOW: {
					uint16_t RelocData;
					GetBinaryData(OffsetRelocData, 0, RelocData);
					RelocData += LOWORD(Delta);
					SetBinaryData(OffsetRelocData, 0, RelocData);
					break;
				}
				case IMAGE_REL_BASED_ABSOLUTE:
					break;
				default:
					strMsg = (std::string)"Unknown base relocation type"e;
					strMsg += std::to_string(RelocType);
					return false;
				}
			}

			nBytes += BaseRelocation.SizeOfBlock;
			OffsetBaseRelocation += sizeof(IMAGE_BASE_RELOCATION) + NumRelocs * sizeof(uint16_t);
		}

		return true;
	}

	bool InsertBinary(size_t OffsetNtHeader, const IMAGE_NT_HEADERS& ntHd, uintptr_t RemoteImageBase) const {
		DWORD dwOldProtect;

		//PE Header
		if (!process.RemoteWriteProcessMemory((void*)RemoteImageBase, &Binary[0], ntHd.OptionalHeader.SizeOfHeaders, 0))
			return false;
		if (!process.RemoteVirtualProtect((void*)RemoteImageBase, ntHd.OptionalHeader.SizeOfHeaders, PAGE_EXECUTE_READ, &dwOldProtect))
			return false;

		//Other Sections
		const size_t OffsetImageSectionHeader = GetOffsetOptionalHeader(OffsetNtHeader, ntHd);

		for (unsigned i = 0; i < ntHd.FileHeader.NumberOfSections; i++) {
			IMAGE_SECTION_HEADER ImageSectionHeader;
			GetBinaryData(OffsetImageSectionHeader, i, ImageSectionHeader);

			if (!ImageSectionHeader.Misc.VirtualSize)
				continue;
			
			if (ImageSectionHeader.SizeOfRawData) {
				if (ImageSectionHeader.PointerToRawData + ImageSectionHeader.SizeOfRawData > Binary.size())
					return false;
				if (!process.RemoteWriteProcessMemory((void*)(RemoteImageBase + ImageSectionHeader.VirtualAddress),
					&Binary[ImageSectionHeader.PointerToRawData], ImageSectionHeader.SizeOfRawData, 0))
					return false;
			}

			DWORD dwProtect;
			switch (ImageSectionHeader.Characteristics & 0xF0000000) {
			case IMAGE_SCN_MEM_EXECUTE:
				dwProtect = PAGE_EXECUTE;
				break;
			case IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ:
				dwProtect = PAGE_EXECUTE_READ;
				break;
			case IMAGE_SCN_MEM_READ:
				dwProtect = PAGE_READONLY;
				break;
			case IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE:
				dwProtect = PAGE_READWRITE;
				break;
			default:
				dwProtect = PAGE_EXECUTE_READWRITE;
				break;
			}

			if (!process.RemoteVirtualProtect((void*)(RemoteImageBase + ImageSectionHeader.VirtualAddress),
				ImageSectionHeader.Misc.VirtualSize, dwProtect, &dwOldProtect))
				return false;
		}

		return true;
	}

	bool AddExceptionTable(size_t OffsetNtHeader, const IMAGE_NT_HEADERS& ntHd, uintptr_t RemoteImageBase) const {
		if (!ntHd.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size)
			return true;

		const size_t OffsetRuntimeFunction = GetOffsetFromRVA(OffsetNtHeader, ntHd,
			ntHd.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
		const size_t Count = 
			ntHd.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size / sizeof(RUNTIME_FUNCTION);

		bool Result = false;
		return process.RemoteCall(RtlAddFunctionTable, &Result, 
			RemoteImageBase + OffsetRuntimeFunction, Count, RemoteImageBase, 0, true, false) && Result;
	}

	bool CallTLSCallbacks(size_t OffsetNtHeader, const IMAGE_NT_HEADERS& ntHd, uintptr_t RemoteImageBase, bool& IsTLSCallbackExist) const {
		IsTLSCallbackExist = false;

		if (!ntHd.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size)
			return true;

		size_t OffsetTlsDirectory = GetOffsetFromRVA(OffsetNtHeader, ntHd,
			ntHd.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);

		IMAGE_TLS_DIRECTORY TlsDirectory;
		GetBinaryData(OffsetTlsDirectory, 0, TlsDirectory);

		for (int i = 0, j = 0; i < 5; i++);

		void* pCallBack = 0;
		for (void** AddressOfCallback = (void**)TlsDirectory.AddressOfCallBacks;
			process.RemoteReadProcessMemory(AddressOfCallback, &pCallBack, sizeof(pCallBack), 0) && pCallBack;
			AddressOfCallback++)
		{
			if (!process.RemoteCall(pCallBack, 0, RemoteImageBase, DLL_PROCESS_ATTACH, 0, 0, true, false))
				return false;
			IsTLSCallbackExist = true;
		}

		return true;
	}

	HMODULE MapRemoteModuleA(bool bCallEntryPoint, const char* szParam,
		EInjectionType InjectionType, const char* szIntoDLL, const char* szDLLDir) {

		size_t OffsetNtHeader;
		IMAGE_NT_HEADERS ntHd;
		if (!CheckHeader(OffsetNtHeader, ntHd))
			return 0;

		const size_t BinaryImageSize = ntHd.OptionalHeader.SizeOfImage;
		const bool IsDLL = (ntHd.FileHeader.Characteristics & IMAGE_FILE_DLL);
		const bool IsRelocatable = (ntHd.OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE);
		void* pRemoteImageBase = 0;

		switch (InjectionType) {
		case EInjectionType::Normal:
		case EInjectionType::NxBitSwap: {
			void* pAllocAddress = IsRelocatable ? 0 : (void*)ntHd.OptionalHeader.ImageBase;
			pRemoteImageBase = process.RemoteVirtualAlloc(pAllocAddress, BinaryImageSize,
				MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			if (!pRemoteImageBase || (pAllocAddress && pAllocAddress != pRemoteImageBase)) {
				strMsg = (std::string)"RemoteVirtualAlloc failed"e;
				return 0;
			}
			break;
		}
		case EInjectionType::IntoDLL: {
			if (!IsRelocatable) {
				strMsg = (std::string)"Image is not relocatable"e;
				return 0;
			}
			if (!szIntoDLL || !strlen(szIntoDLL)) {
				strMsg = (std::string)"No DLL specified"e;
				return 0;
			}
			if (RemoteGetModuleHandleA(szIntoDLL)) {
				strMsg = szIntoDLL;
				strMsg += (const char*)" is already loaded in target process"e;
				return 0;
			}

			pRemoteImageBase = RemoteLoadLibraryExA(szIntoDLL, DONT_RESOLVE_DLL_REFERENCES);
			if (!pRemoteImageBase) {
				strMsg = (std::string)"RemoteLoadLibrary failed : "e;
				strMsg += szIntoDLL;
				return 0;
			}
			pRemoteImageBase = (void*)((uintptr_t)pRemoteImageBase + 0x1000);

			MEMORY_BASIC_INFORMATION MemInfo;
			if (!process.RemoteVirtualQuery(pRemoteImageBase, &MemInfo, sizeof(MemInfo))) {
				strMsg = (std::string)"VirtualQueryEx Failed"e;
				return 0;
			}

			if (MemInfo.RegionSize < BinaryImageSize) {
				strMsg = (std::string)"Not enough .text section size\nCurrent size : "e;
				strMsg += std::to_string(MemInfo.RegionSize);
				strMsg += (const char*)"\nRequired size : "e;
				strMsg += std::to_string(BinaryImageSize);
				return 0;
			}

			if (!process.BypassCFG()) {
				strMsg = (std::string)"BypassCFG Failed"e;
				return 0;
			}
			break;
		}
		default: return 0;
		}

		if (!FixImports(OffsetNtHeader, ntHd, szDLLDir))
			return 0;

		if (!FixRelocs(OffsetNtHeader, ntHd, (uintptr_t)pRemoteImageBase))
			return 0;

		std::vector<uint8_t> ZeroFill(BinaryImageSize, 0);
		if (!process.RemoteWriteProcessMemory(pRemoteImageBase, ZeroFill.data(), BinaryImageSize, 0)) {
			strMsg = (std::string)"Initializing space failed"e;
			return 0;
		}

		if (!InsertBinary(OffsetNtHeader, ntHd, (uintptr_t)pRemoteImageBase)) {
			strMsg = (std::string)"Inserting binary failed"e;
			return 0;
		}

		if (!bCallEntryPoint)
			return (HMODULE)pRemoteImageBase;

		//if (!AddExceptionTable(OffsetNtHeader, ntHd, (uintptr_t)pRemoteImageBase)) {
		//	strMsg = (std::string)"Calling RtlAddFunctionTable failed"e;
		//	return 0;
		//}

		bool IsTLSCallbackExist = false;
		if (!CallTLSCallbacks(OffsetNtHeader, ntHd, (uintptr_t)pRemoteImageBase, IsTLSCallbackExist)) {
			strMsg = (std::string)"Calling TLS Callback failed"e;
			return 0;
		}

		//Calling entrypoint
		bool bSuccess = [&]{
			void* pszParam = 0;
			if (szParam && strlen(szParam)) {
				const size_t Len = strlen(szParam) + 1;
				pszParam = process.RemoteVirtualAlloc(0, Len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
				if (!pszParam)
					return false;
				if (!process.RemoteWriteProcessMemory(pszParam, szParam, Len, 0))
					return false;
			}

			void* pRemoteEntryPoint = (void*)((uintptr_t)pRemoteImageBase + ntHd.OptionalHeader.AddressOfEntryPoint);

			if (IsDLL)
				return process.RemoteCall(pRemoteEntryPoint, 0, pRemoteImageBase, DLL_PROCESS_ATTACH, pszParam, 0, false, true);

			//EXE
			return process.RemoteCall(pRemoteEntryPoint, 0, pRemoteImageBase, 0, pszParam, SW_SHOW, false, true);
		}();

		if (!bSuccess) {
			strMsg = (std::string)"Calling entrypoint failed"e;
			return 0;
		}

		if (InjectionType == EInjectionType::NxBitSwap) {
			//Waiting to unpack
			Sleep(500);
			if (!process.RemoveNXBit(pRemoteImageBase, BinaryImageSize)) {
				strMsg = (std::string)"NxBit"e;
				return 0;
			}
		}

		return (HMODULE)pRemoteImageBase;
	}

public:
	std::string GetErrorMsg() const { return strMsg; }

	Injector(const RemoteProcess& process) : process(process) {}
	~Injector() {}

	HMODULE MapRemoteModuleAFromFileName(const char* szModule, bool bCallDLLMain,
		const char* szParam, EInjectionType InjectionType, const char* szIntoDLL) {

		HANDLE hFile = CreateFileA(szModule, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile == INVALID_HANDLE_VALUE)
			return 0;

		DWORD dwFileSize = GetFileSize(hFile, 0);
		if (!dwFileSize)
			return 0;

		Binary.resize(dwFileSize);
		if (!ReadFile(hFile, Binary.data(), dwFileSize, 0, 0))
			return 0;

		char szDLLPath[MAX_PATH];
		GetFinalPathNameByHandleA(hFile, szDLLPath, MAX_PATH, FILE_NAME_OPENED);
		std::string strDLLDir = szDLLPath;
		strDLLDir = strDLLDir.substr(0, strDLLDir.find_last_of((const char*)"\\\\"e));

		CloseHandle(hFile);

		return MapRemoteModuleA(bCallDLLMain, szParam, InjectionType, szIntoDLL, strDLLDir.c_str());
	}
};
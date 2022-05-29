#pragma once
#include <windows.h>
#include <tlhelp32.h>

#include "common/kernel.h"
#include "peb.h"

class Process {
private:
	uintptr_t BaseAddress = 0;
	uintptr_t SizeOfImage = 0;
	DWORD Pid = 0;

	bool LoadCurrentModuleToRam() const {
		HANDLE hProcess = ::OpenProcess(PROCESS_VM_READ, false, Pid);
		if (!hProcess)
			return false;

		bool bSuccess = true;
		for (uintptr_t i = 0; i < SizeOfImage; i += 0x1000) {
			uint8_t byte;
			if (!::ReadProcessMemory(hProcess, (void*)(BaseAddress + i), &byte, 1, 0)) {
				bSuccess = false;
				break;
			}
		}
		CloseHandle(hProcess);
		return bSuccess;
	}
	
public:
	Kernel& kernel;

	Process(Kernel& kernel, DWORD Pid, const char* szDLLName = 0) : kernel(kernel), Pid(Pid) {
		if (!kernel.MapProcess(Pid))
			throw;
		if (!SetModule(szDLLName))
			throw;
	}

	uintptr_t GetBaseAddress() const { return BaseAddress; }
	uintptr_t GetSizeOfImage() const { return SizeOfImage; }
	DWORD GetPid() const { return Pid; }

	const tReadProcessMemory ReadProcessMemory = [&](uintptr_t Address, void* Buffer, uintptr_t Size) {
		return kernel.ReadProcessMemoryFast(Address, Buffer, Size);
	};
	const tWriteProcessMemory WriteProcessMemory = [&](uintptr_t Address, const void* Buffer, uintptr_t Size) {
		return kernel.WriteProcessMemoryFast(Address, Buffer, Size);
	};

	template <class T>
	bool Read(uintptr_t Address, T* Buffer) const { return ReadProcessMemory(Address, Buffer, sizeof(T)); }
	template <class T>
	bool Write(uintptr_t Address, const T* Buffer) const { return WriteProcessMemory(Address, Buffer, sizeof(T)); }

	template <class T>
	bool ReadBase(uintptr_t Address, T* Buffer) const { return Read(BaseAddress + Address, Buffer); }
	template <class T>
	bool WriteBase(uintptr_t Address, const T* Buffer) const { return Write(BaseAddress + Address, Buffer); }

	bool SetModule(const char* szModuleName = 0) {
		std::wstring wModuleName;
		if (szModuleName)
			wModuleName = std::wstring(szModuleName, szModuleName + strlen(szModuleName));
	
		const uintptr_t PebBaseAddress = kernel.GetPebAddress(Pid);
		if (!PebBaseAddress)
			return false;

		PEB64 Peb64;
		if (!Read(PebBaseAddress, &Peb64))
			return false;

		PEB_LDR_DATA64 ldr_data;
		if (!Read(Peb64.LoaderData, &ldr_data))
			return false;

		uintptr_t pldr = ldr_data.InLoadOrderModuleList.Flink;
		do {
			LDR_DATA_TABLE_ENTRY64 ldr_entry;
			if (!Read(pldr, &ldr_entry))
				break;

			wchar_t wBaseModuleName[0x100] = { 0 };
			if (ReadProcessMemory(ldr_entry.BaseDllName.Buffer, wBaseModuleName, ldr_entry.BaseDllName.Length * sizeof(wchar_t))) {
				if (wModuleName.size() == 0 || wModuleName == wBaseModuleName) {
					BaseAddress = ldr_entry.BaseAddress;
					SizeOfImage = ldr_entry.SizeOfImage;
					return true;
				}
			}

			pldr = ldr_entry.InLoadOrderModuleList.Flink;
		} while (pldr != ldr_data.InLoadOrderModuleList.Flink);

		return false;
	}

	uintptr_t ScanRange(uintptr_t BaseAddress, size_t Len, const char* szPattern) const {
		return PatternScan::Range(BaseAddress, Len, szPattern, ReadProcessMemory);
	}

	uintptr_t ScanCurrentModule(const char* szPattern, const char* szSectionName = ".text"e) const {
		LoadCurrentModuleToRam();
		return PatternScan::Module(BaseAddress, szSectionName, szPattern, ReadProcessMemory);
	}
};
#pragma once
#include <Windows.h>
#include "function_ref.hpp"

class PatternScan {
private:
	template<class Type>
	using tReadProcessMemory = tl::function<bool(Type Address, void* Buffer, size_t Size)>;

	constexpr static unsigned MAX_MASK_SIZE = 0x100;

	static bool DataCompare(uint8_t* pData, bool* bMask, uint8_t* vMask, size_t SizeMask) {
		for (auto i = 0; i < SizeMask; i++) {
			if (bMask && !bMask[i])
				continue;
			if (pData[i] != vMask[i])
				return false;
		}
		return true;
	}

	static uintptr_t FindPattern(uintptr_t Address, size_t Len, bool* bMask, uint8_t* vMask, size_t SizeMask) {
		for (size_t i = 0; i < Len; i++)
			if (DataCompare((uint8_t*)(Address + i), bMask, vMask, SizeMask))
				return Address + i;
		return 0;
	}

	static size_t ConvertStringToMask(const char* szPattern, bool* bMask, uint8_t* vMask, size_t MaxSize) {
		memset(bMask, 0, MaxSize);
		memset(vMask, 0, MaxSize);
		uintptr_t i = 0, j = 0, k = 0;
		while (szPattern[i]) {
			if (szPattern[i] == ' ') {
				k = 0;
				j++;
				if (j >= MaxSize)
					return 0;
			}
			else if (szPattern[i] >= '0' && szPattern[i] <= '9') {
				bMask[j] = true;
				if (k == 1) vMask[j] += szPattern[i] - '0';
				else if (k == 0) vMask[j] += (szPattern[i] - '0') * 0x10;
				else return 0;
				k++;
			}
			else if (szPattern[i] >= 'a' && szPattern[i] <= 'f') {
				bMask[j] = true;
				if (k == 1) vMask[j] += szPattern[i] - 'a' + 0xA;
				else if (k == 0) vMask[j] += (szPattern[i] - 'a' + 0xA) * 0x10;
				else return 0;
				k++;
			}
			else if (szPattern[i] >= 'A' && szPattern[i] <= 'F') {
				bMask[j] = true;
				if (k == 1) vMask[j] += szPattern[i] - 'A' + 0xA;
				else if (k == 0) vMask[j] += (szPattern[i] - 'A' + 0xA) * 0x10;
				else return 0;
				k++;
			}
			else if (szPattern[i] == '?') {
				bMask[j] = false;
				vMask[j] = 0;
			}
			else return 0;
			i++;
		}
		return j + 1;
	}

	template <class Type>
	static bool GetModuleInfo(Type ModuleBase, const char* szSectionName, DWORD& Base, DWORD& Size, tReadProcessMemory<Type> ReadProcessMemory) {
		static_assert(std::is_same<Type, uint32_t>::value | std::is_same<Type, uint64_t>::value, "Type must be 32bit or 64bit.");

		IMAGE_DOS_HEADER DosHeader;
		if (!ReadProcessMemory(ModuleBase, &DosHeader, sizeof(DosHeader)))
			return false;

		if (DosHeader.e_magic != IMAGE_DOS_SIGNATURE)
			return false;

		using NtHeadersType = IMAGE_NT_HEADERS64;
		if (std::is_same<Type, uint32_t>::value)
			using NtHeadersType = IMAGE_NT_HEADERS32;

		NtHeadersType NtHeaders;
		if (!ReadProcessMemory(ModuleBase + DosHeader.e_lfanew, &NtHeaders, sizeof(NtHeaders)))
			return false;

		if (NtHeaders.Signature != IMAGE_NT_SIGNATURE)
			return false;

		if (!szSectionName) {
			Base = 0;
			Size = NtHeaders.OptionalHeader.SizeOfImage;
			return true;
		}

		for (unsigned i = 0; i < NtHeaders.FileHeader.NumberOfSections; i++) {
			uintptr_t SectionPtr =
				ModuleBase + DosHeader.e_lfanew +
				offsetof(NtHeadersType, OptionalHeader) +
				NtHeaders.FileHeader.SizeOfOptionalHeader +
				sizeof(IMAGE_SECTION_HEADER) * i;

			IMAGE_SECTION_HEADER Section;
			if (!ReadProcessMemory(SectionPtr, &Section, sizeof(Section)))
				continue;

			char NameNullTerminated[sizeof(Section.Name) + 1] = { 0 };
			memcpy(NameNullTerminated, Section.Name, sizeof(Section.Name));

			if (strcmp(szSectionName, NameNullTerminated) != 0)
				continue;

			Base = Section.VirtualAddress;
			Size = Section.Misc.VirtualSize;
			return true;
		}

		return false;
	}

public:
	template <class Type>
	static Type Range(Type Address, size_t Len, bool* bMask, uint8_t* vMask, size_t SizeMask, tReadProcessMemory<Type> ReadProcessMemory) {
		static_assert(std::is_same<Type, uint32_t>::value | std::is_same<Type, uint64_t>::value, "Type must be 32bit or 64bit.");

		Type Start = Address & ~0xFFF;
		Type End = (Address + Len + 0x1000) & ~0xFFF;

		for (Type i = Start; i < End; i += 0x1000) {
			uint8_t Buf[0x2000];

			if (!ReadProcessMemory(i, Buf, 0x1000))
				continue;

			ReadProcessMemory(i + 0x1000, Buf + 0x1000, 0x1000);

			Type AddressTemp = (Type)Buf;
			while (1) {
				Type LocalResult = FindPattern(AddressTemp, 0x1000, bMask, vMask, SizeMask);
				if (LocalResult == 0)
					break;
				else {
					Type Result = LocalResult - (Type)Buf + i;
					if (Result >= Address && Result < Address + Len && Result != (Type)vMask)
						return Result;

					AddressTemp = LocalResult + 1;
				}
			}
		}
		return 0;
	}

	template <class Type>
	static Type Range(Type Address, size_t Len, const char* szPattern, tReadProcessMemory<Type> ReadProcessMemory) {
		static_assert(std::is_same<Type, uint32_t>::value | std::is_same<Type, uint64_t>::value, "Type must be 32bit or 64bit.");

		bool bMask[MAX_MASK_SIZE] = { 0 };
		uint8_t vMask[MAX_MASK_SIZE] = { 0 };
		size_t SizeMask = ConvertStringToMask(szPattern, bMask, vMask, MAX_MASK_SIZE);
		if (!SizeMask)
			return 0;

		return Range(Address, Len, bMask, vMask, SizeMask, ReadProcessMemory);
	}

	template <class Type>
	static Type Module(Type ModuleBase, const char* szSectionName, bool* bMask, uint8_t* vMask, size_t SizeMask, tReadProcessMemory<Type> ReadProcessMemory) {
		static_assert(std::is_same<Type, uint32_t>::value | std::is_same<Type, uint64_t>::value, "Type must be 32bit or 64bit.");

		DWORD Base, Size;
		if (!GetModuleInfo<Type>(ModuleBase, szSectionName, Base, Size, ReadProcessMemory))
			return 0;

		return Range(ModuleBase + Base, Size, bMask, vMask, SizeMask, ReadProcessMemory);
	}

	template <class Type>
	static Type Module(Type ModuleBase, const char* szSectionName, const char* szPattern, tReadProcessMemory<Type> ReadProcessMemory) {
		static_assert(std::is_same<Type, uint32_t>::value | std::is_same<Type, uint64_t>::value, "Type must be 32bit or 64bit.");

		DWORD Base, Size;
		if (!GetModuleInfo<Type>(ModuleBase, szSectionName, Base, Size, ReadProcessMemory))
			return 0;

		return Range(ModuleBase + Base, Size, szPattern, ReadProcessMemory);
	}

	static uintptr_t Range(uintptr_t Address, size_t Len, bool* bMask, uint8_t* vMask, size_t SizeMask, tReadProcessMemory<uintptr_t> ReadProcessMemory) {
		return Range<uintptr_t>(Address, Len, bMask, vMask, SizeMask, ReadProcessMemory);
	}

	static uintptr_t Range(uintptr_t Address, size_t Len, const char* szPattern, tReadProcessMemory<uintptr_t> ReadProcessMemory) {
		return Range<uintptr_t>(Address, Len, szPattern, ReadProcessMemory);
	}

	static uintptr_t Module(uintptr_t ModuleBase, const char* szSectionName, bool* bMask, uint8_t* vMask, size_t SizeMask, tReadProcessMemory<uintptr_t> ReadProcessMemory) {
		return Module<uintptr_t>(ModuleBase, szSectionName, bMask, vMask, SizeMask, ReadProcessMemory);
	}

	static uintptr_t Module(uintptr_t ModuleBase, const char* szSectionName, const char* szPattern, tReadProcessMemory<uintptr_t> ReadProcessMemory) {
		return Module<uintptr_t>(ModuleBase, szSectionName, szPattern, ReadProcessMemory);
	}

	static uintptr_t GetJumpAddress(uintptr_t OpcodeAddress, tReadProcessMemory<uintptr_t> ReadProcessMemory) {
		uint8_t Opcode[5];
		if (!ReadProcessMemory(OpcodeAddress, Opcode, 5))
			return 0;
		return uintptr_t(intptr_t(OpcodeAddress + 0x5) + *(int*)(Opcode + 1));
	}
};
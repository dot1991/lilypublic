#pragma once
#include <Windows.h>

#include "common/ida_defs.h"
#include "pubg_process.h"
#include "pubg_struct.h"

class TNameEntryArray;

class UObject
{
private:
	uint8_t pad[0x40];
public:
	//DWORD64 GetClass() {
	//	__int64 a1 = (__int64)this;
	//	__int64 v2; // rax

	//	v2 = __ROL8__(*(_QWORD*)(a1 + 0x18) ^ 0x7753CBD5413FEAD7i64, 0x17);
	//	return v2 ^ (v2 << 0x20) ^ 0xA7950E40FAB9478i64;
	//}

	FName GetFName() const {
		__int64 a1 = (__int64)this;
		int v1;
		int v2;
		int v3;
		int v4;
		__int64 v71;

		v1 = *(_DWORD*)(a1 + 0x30) ^ 0x39F1F469;
		v2 = __ROR4__(*(_DWORD*)(a1 + 0x2C) ^ 0x472BDFE6, 1);
		v3 = v2 ^ (v2 << 0x10) ^ 0xCD98F469;
		LODWORD(v71) = v3;
		v4 = __ROL4__(v1, 7);
		HIDWORD(v71) = v4 ^ (v4 << 0x10) ^ 0x98CDDFE6;

		return *(FName*)&v71;
	}

	//DWORD GetIndex() {
	//	__int64 a2 = (__int64)this;
	//	int v2;

	//	v2 = __ROL4__(*(_DWORD*)(a2 + 8) ^ 0x45D2C49B, 9);
	//	return v2 ^ (v2 << 0x10) ^ 0x5C80CBD3;
	//}

	//DWORD GetObjectFlags() {
	//	unsigned __int64 v2 = (unsigned __int64)this;
	//	int v3;

	//	v3 = __ROR4__(*(_DWORD*)(v2 + 0x30) ^ 0xD1911304, 8);
	//	return (v3 ^ (v3 << 0x10) ^ 0x354055CB);
	//}

	//DWORD64 GetOuter() {
	//	unsigned __int64 v2 = (unsigned __int64)this;
	//	__int64 v4;

	//	v4 = __ROR8__(*(_QWORD*)(v2 + 0x28) ^ 0x1555DE0E8CFD7EE3i64, 3);
	//	v2 = v4 ^ (v4 << 0x20) ^ 0x3811FD43E3F5A7C4i64;
	//	return v2;
	//}
};

class FUObjectItem
{
private:	uint8_t Pad1[0x8];
public:		uintptr_t ObjectPtr;
private:	uint8_t Pad2[0x20];
};

class FUObjectArray
{
private:
	static constexpr uintptr_t ADDRESS_GOBJECTS = 0x8F70468;

	uintptr_t BaseAddress;
	uint32_t NumElements;

public:
	FUObjectArray() {
		EncryptedPtr<uintptr_t> P;
		g_Pubg->ReadBase(ADDRESS_GOBJECTS + 0x18, &P);
		BaseAddress = P;
		g_Pubg->ReadBase(ADDRESS_GOBJECTS + 0x28, &NumElements);
	}

	NativePtr<UObject> GetNativePtrById(size_t Index) const {
		if (Index >= NumElements)
			return 0;

		FUObjectItem ObjectItem;
		if (g_Pubg->Read(BaseAddress + Index * sizeof(ObjectItem), &ObjectItem) == 0)
			return 0;

		return ObjectItem.ObjectPtr;
	}

	size_t GetNumElements() const { return NumElements; }
	void DumpObject() const {
		for (unsigned i = 0; i < GetNumElements(); i++) {
			UObject obj;
			auto Ptr = GetNativePtrById(i);
			if (!Ptr.Read(obj))
				continue;

			std::string Name = obj.GetFName().GetName();
			if (Name.empty())
				continue;

			if (Name == (std::string)"World"e)
				dprintf("%I64X %s\n"e, (uintptr_t)Ptr, Name.c_str());
			if (Name == (std::string)"PlayerState"e)
				dprintf("%I64X %s\n"e, (uintptr_t)Ptr, Name.c_str());
			if (Name == (std::string)"ForceLayoutPrepass"e)
				dprintf("%I64X %s\n"e, (uintptr_t)Ptr, Name.c_str());
		}
	}
};
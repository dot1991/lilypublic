#include "GNames.h"
#include "GObjects.h"
#include "pubg_struct.h"

TNameEntryArray::TNameEntryArray() {
	EncryptedPtr<uintptr_t> P;
	g_Pubg->ReadBase(ADDRESS_GNAMES, &P);
	g_Pubg->Read(P, &P);
	g_Pubg->Read(P, &P);
	//////////////////////////////////////////////////////////////////
	EncryptedPtr<uintptr_t> P2;
	g_Pubg->Read(P + 0, &P2); BasePtr = P2;
	//gXenuine->process.GetValue(P + 8, &P2); NumElements = P2;
}

//DWORD64 GetNumElements() const { return NumElements; }

bool TNameEntryArray::GetName(FName ID, char* szBuf, size_t SizeMax) const {
	if (ID.ComparisonIndex <= 0)
		return false;

	uintptr_t Ptr = BasePtr + sizeof(UINT_PTR) * (ID.ComparisonIndex / ElementsPerChunk);
	g_Pubg->Read(Ptr, &Ptr);
	if (!Ptr)
		return false;

	Ptr = Ptr + sizeof(UINT_PTR) * (ID.ComparisonIndex % ElementsPerChunk);
	g_Pubg->Read(Ptr, &Ptr);
	if (!Ptr)
		return false;

	if (!g_Pubg->ReadProcessMemory(Ptr + offsetof(FNameEntry, AnsiName), szBuf, std::clamp(SizeMax, (size_t)0, NAME_SIZE)))
		return false;

	return true;
}

void TNameEntryArray::EnumNames(tl::function<bool(FName ID, const char* szName)> f) const {
	for (int Index = 1; Index < 0x100000; Index++) {
		char szBuf[NAME_SIZE];
		if (GetName({ (int)Index, 0 }, szBuf, sizeof(szBuf)) && !f({ Index, 0 }, szBuf))
			return;
	}
}

FName TNameEntryArray::FindName(const char* szNameToFind) const {
	FName Result = {};
	EnumNames([&](FName ID, const char* szName) {
		if (strcmp(szNameToFind, szName) != 0)
			return true;
		Result = ID;
		return false;
		});
	return Result;
}

void TNameEntryArray::DumpAllNames() const {
	FILE* out = fopen("out.txt"e, "w"e);

	EnumNames([&](FName ID, const char* szName) {
		fprintf(out, "%06X %s\n"e, ID.ComparisonIndex, szName);
		return true;
		});

	fclose(out);
	verify(0);
}
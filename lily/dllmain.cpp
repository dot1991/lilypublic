#include <Windows.h>
#include "global.h"

void realmain();

bool ErasePE(HMODULE hDLL) {
	uint8_t RandBytes[0x1000];
	srand(GetCurrentThreadId());
	for (auto& i : RandBytes)
		i = (uint8_t)rand();

	return WriteProcessMemory((HANDLE)-1, hDLL, RandBytes, 0x1000, 0);
}

BOOL WINAPI DllMain(HMODULE hDLL, DWORD dwReason, char* szParam) {
	if (dwReason != DLL_PROCESS_ATTACH)
		return FALSE;

	if (szParam)
		VirtualFree(szParam, 0, MEM_RELEASE);

	const bool IsInjectorValid = [&] {
		if (!ErasePE(hDLL))
			return false;

		MEMORY_BASIC_INFORMATION MemInfo;
		for (auto i = 0; i < 20; i++, Sleep(100)) {
			if (!VirtualQuery(DllMain, &MemInfo, sizeof(MemInfo)))
				return false;
			if (MemInfo.Protect == PAGE_READWRITE)
				break;
		}

		if (MemInfo.Protect != PAGE_READWRITE)
			return false;

		const uintptr_t Base = (uintptr_t)MemInfo.AllocationBase;
		const uintptr_t Size = (uintptr_t)MemInfo.BaseAddress - (uintptr_t)MemInfo.AllocationBase + MemInfo.RegionSize;
		if (Base != (uintptr_t)hDLL)
			return false;

		Global::SetModuleInfo(Base, Size);
		return true;
	}();

	verify(IsInjectorValid);
	realmain();
}
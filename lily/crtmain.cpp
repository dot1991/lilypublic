#include <Windows.h>
#include "global.h"

void realmain();

int main() {
	MODULEINFO ModuleInfo;
	GetModuleInformation((HANDLE)-1, GetModuleHandleA(0), &ModuleInfo, sizeof(ModuleInfo));
	Global::SetModuleInfo(ModuleInfo.lpBaseOfDll, ModuleInfo.SizeOfImage);
	realmain();
	return 0;
}
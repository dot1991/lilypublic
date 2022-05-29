#pragma once
#include "common/util.h"
#include "common/initializer.h"
#include "common/encrypt_string.h"

class Global {
public:
	static void SetModuleInfo(auto Base, auto Size) {
		ModuleBase = (uintptr_t)Base;
		ModuleSize = (uintptr_t)Size;
	}
private:
	static inline uintptr_t ModuleBase = 0;
	static inline size_t ModuleSize = 0;

	INITIALIZER_INCLASS(UEF) {
		SetUnhandledExceptionFilter([](PEXCEPTION_POINTERS pExceptionInfo)->LONG {
			const uintptr_t ExceptionAddress = (uintptr_t)pExceptionInfo->ExceptionRecord->ExceptionAddress;
			char szAddress[0x40];
			if (ModuleBase && ExceptionAddress >= ModuleBase && ExceptionAddress < ModuleBase + ModuleSize)
				sprintf(szAddress, "Base+0x%I64X"e, ExceptionAddress - ModuleBase);
			else
				sprintf(szAddress, "0x%I64X"e, ExceptionAddress);

			error(szAddress, "Unhandled exception"e);
			return EXCEPTION_CONTINUE_SEARCH;
			});
	};

	INITIALIZER_INCLASS(EnableDebugPrivilege) {
		bool bSuccess = false;
		[&] {
			HANDLE hToken;
			if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken))
				return;

			bSuccess = SetPrivilege(hToken, "SeDebugPrivilege"e, true);
			CloseHandle(hToken);
		}();

		verify(bSuccess);
	};
};
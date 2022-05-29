#pragma once
#include "common/util.h"
#include "common/initializer.h"
#include "common/encrypt_string.h"
#include "common/dbvm.h"

class Global {
public:
	static inline char DBVMPassword[21];
	static inline DBVM dbvm;

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

	INITIALIZER_INCLASS(InitDBVM) {
		DBVMPassword << "qS2n9TLRX8iZ9YmGqgsK"e;

		uint64_t password1 = *(uint64_t*)(Global::DBVMPassword + 0) ^ 0xda2355698be6166c;
		uint32_t password2 = *(uint32_t*)(Global::DBVMPassword + 8) ^ 0x6765fa70;
		uint64_t password3 = *(uint64_t*)(Global::DBVMPassword + 12) ^ 0xe21cb5155c065962;

		dbvm.SetPassword(password1, password2, password3);
		verify(dbvm.GetVersion());
		dbvm.CloakReset();
	};
};
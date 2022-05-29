#pragma once
#include <windows.h>
#include "common/initializer.h"

class ExceptionHandler {
private:
	thread_local static inline DWORD* pExceptionCode;
	thread_local static inline CONTEXT* pCapturedContext;
	thread_local static inline DWORD LastExceptionCode;

	INITIALIZER_INCLASS(VEH) {
		AddVectoredExceptionHandler(1, [](PEXCEPTION_POINTERS pExceptionInfo)->LONG {
			if (pExceptionCode == 0 || *pExceptionCode != 0)
				return EXCEPTION_CONTINUE_SEARCH;

			*pExceptionCode = pExceptionInfo->ExceptionRecord->ExceptionCode;
			*pExceptionInfo->ContextRecord = *pCapturedContext;
			return EXCEPTION_CONTINUE_EXECUTION;
		});
	};

public:
	static DWORD GetLastExceptionCode() { return LastExceptionCode; }
	static bool TryExcept(auto pFunc) {
		volatile DWORD ExceptionCode = 0;
		CONTEXT CapturedContext;

		DWORD* pPrevExceptionCode = pExceptionCode;
		CONTEXT* pPrevCapturedContext = pCapturedContext;

		pExceptionCode = (DWORD*)&ExceptionCode;
		pCapturedContext = &CapturedContext;

		RtlCaptureContext(&CapturedContext);

		//return false when exception accured
		if (ExceptionCode) {
			LastExceptionCode = ExceptionCode;
			pExceptionCode = pPrevExceptionCode;
			pCapturedContext = pPrevCapturedContext;
			return false;
		}
			
		pFunc();

		LastExceptionCode = 0;
		pExceptionCode = pPrevExceptionCode;
		pCapturedContext = pPrevCapturedContext;
		return true;
	}
};
#pragma once
#include <Windows.h>
#include <bcrypt.h>		//NTSTATUS
#include <SubAuth.h>	//UNICODE_STRING

typedef enum _HARDERROR_RESPONSE_OPTION {
	OptionAbortRetryIgnore,
	OptionOk,
	OptionOkCancle,
	OptionRetryCancle,
	OptionYesNo,
	OptionYesNoCancle,
	OptionShutdownSystem,
	OptionOkNoWait,
	OptionCancelTryContinue
}HARDERROR_RESPONSE_OPTION, * PHARDERROR_RESPONSE_OPTION;

typedef enum _HARDERROR_RESPONSE {
	ResponseReturnToCaller,
	ResponseNotHandled,
	ResponseAbort,
	ResponseCancel,
	ResponseIgnore,
	ResponseNo,
	ResponseOk,
	ResponseRetry,
	ResponseYes
} HARDERROR_RESPONSE, * PHARDERROR_RESPONSE;

using tNtRaiseHardError = NTSTATUS(*)(
	IN NTSTATUS ErrorStatus, 
	IN ULONG NumberOfParameters,
	IN ULONG UnicodeStringParameterMask OPTIONAL,
	IN ULONG_PTR* Parameters, 
	IN HARDERROR_RESPONSE_OPTION ResponseOption, 
	OUT PHARDERROR_RESPONSE Response);

#define STATUS_SERVICE_NOTIFICATION_2 0x50000018
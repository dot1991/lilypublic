#pragma once
#include <Windows.h>
#include <bcrypt.h>		//NTSTATUS
#include <SubAuth.h>	//UNICODE_STRING
#include "common/defclass.h"

class OBJECT_TYPE;
typedef OBJECT_TYPE* POBJECT_TYPE;
typedef ULONG OB_OPERATION;

typedef struct _OB_PRE_CREATE_HANDLE_INFORMATION {
	_Inout_ ACCESS_MASK         DesiredAccess;
	_In_ ACCESS_MASK            OriginalDesiredAccess;
} OB_PRE_CREATE_HANDLE_INFORMATION, * POB_PRE_CREATE_HANDLE_INFORMATION;

typedef struct _OB_PRE_DUPLICATE_HANDLE_INFORMATION {
	_Inout_ ACCESS_MASK         DesiredAccess;
	_In_ ACCESS_MASK            OriginalDesiredAccess;
	_In_ PVOID                  SourceProcess;
	_In_ PVOID                  TargetProcess;
} OB_PRE_DUPLICATE_HANDLE_INFORMATION, * POB_PRE_DUPLICATE_HANDLE_INFORMATION;

typedef union _OB_PRE_OPERATION_PARAMETERS {
	_Inout_ OB_PRE_CREATE_HANDLE_INFORMATION        CreateHandleInformation;
	_Inout_ OB_PRE_DUPLICATE_HANDLE_INFORMATION     DuplicateHandleInformation;
} OB_PRE_OPERATION_PARAMETERS, * POB_PRE_OPERATION_PARAMETERS;

typedef struct _OB_POST_CREATE_HANDLE_INFORMATION {
	_In_ ACCESS_MASK            GrantedAccess;
} OB_POST_CREATE_HANDLE_INFORMATION, * POB_POST_CREATE_HANDLE_INFORMATION;

typedef struct _OB_POST_DUPLICATE_HANDLE_INFORMATION {
	_In_ ACCESS_MASK            GrantedAccess;
} OB_POST_DUPLICATE_HANDLE_INFORMATION, * POB_POST_DUPLICATE_HANDLE_INFORMATION;

typedef union _OB_POST_OPERATION_PARAMETERS {
	_In_ OB_POST_CREATE_HANDLE_INFORMATION       CreateHandleInformation;
	_In_ OB_POST_DUPLICATE_HANDLE_INFORMATION    DuplicateHandleInformation;
} OB_POST_OPERATION_PARAMETERS, * POB_POST_OPERATION_PARAMETERS;

typedef struct _OB_PRE_OPERATION_INFORMATION {
	_In_ OB_OPERATION           Operation;
	union {
		_In_ ULONG Flags;
		struct {
			_In_ ULONG KernelHandle : 1;
			_In_ ULONG Reserved : 31;
		};
	};
	_In_ PVOID                         Object;
	_In_ POBJECT_TYPE                  ObjectType;
	_Out_ PVOID                        CallContext;
	_In_ POB_PRE_OPERATION_PARAMETERS  Parameters;
} OB_PRE_OPERATION_INFORMATION, * POB_PRE_OPERATION_INFORMATION;

typedef struct _OB_POST_OPERATION_INFORMATION {
	_In_ OB_OPERATION  Operation;
	union {
		_In_ ULONG Flags;
		struct {
			_In_ ULONG KernelHandle : 1;
			_In_ ULONG Reserved : 31;
		};
	};
	_In_ PVOID                          Object;
	_In_ POBJECT_TYPE                   ObjectType;
	_In_ PVOID                          CallContext;
	_In_ NTSTATUS                       ReturnStatus;
	_In_ POB_POST_OPERATION_PARAMETERS  Parameters;
} OB_POST_OPERATION_INFORMATION, * POB_POST_OPERATION_INFORMATION;

typedef enum _OB_PREOP_CALLBACK_STATUS {
	OB_PREOP_SUCCESS
} OB_PREOP_CALLBACK_STATUS, * POB_PREOP_CALLBACK_STATUS;

typedef VOID
(*POB_POST_OPERATION_CALLBACK) (
	_In_ PVOID RegistrationContext,
	_In_ POB_POST_OPERATION_INFORMATION OperationInformation
	);

typedef OB_PREOP_CALLBACK_STATUS
(*POB_PRE_OPERATION_CALLBACK) (
	_In_ PVOID RegistrationContext,
	_Inout_ POB_PRE_OPERATION_INFORMATION OperationInformation
	);

typedef struct _OB_OPERATION_REGISTRATION {
	_In_ POBJECT_TYPE* ObjectType;
	_In_ OB_OPERATION                Operations;
	_In_ POB_PRE_OPERATION_CALLBACK  PreOperation;
	_In_ POB_POST_OPERATION_CALLBACK PostOperation;
} OB_OPERATION_REGISTRATION, * POB_OPERATION_REGISTRATION;

//////////////////////////Undocumented////////////////////////////////////
//////////////////////////Undocumented////////////////////////////////////
//////////////////////////Undocumented////////////////////////////////////
//https://www.vergiliusproject.com/kernels/x64/Windows%2011/21H2%20(RTM)/_OBJECT_TYPE
DefBaseClass(OBJECT_TYPE,
	MemberAtOffset(LIST_ENTRY, CallbackList, 0xc8)
,)

typedef struct _CALLBACK_ENTRY
{
	INT16							Version;
	unsigned char					unknown[6];
	POB_OPERATION_REGISTRATION		RegistrationContext;
	UNICODE_STRING					Altitude;
} CALLBACK_ENTRY, * PCALLBACK_ENTRY;

typedef struct _CALLBACK_ENTRY_ITEM
{
	LIST_ENTRY						EntryItemList;
	OB_OPERATION					Operations1;
	OB_OPERATION					Operations2;
	PCALLBACK_ENTRY					CallbackEntry;
	POBJECT_TYPE					ObjectType;
	POB_PRE_OPERATION_CALLBACK		PreOperation;
	POB_POST_OPERATION_CALLBACK		PostOperation;
} CALLBACK_ENTRY_ITEM, * PCALLBACK_ENTRY_ITEM;

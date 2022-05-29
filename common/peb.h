#pragma once

#include <Windows.h>

typedef struct _PEB_LDR_DATA64 {
	ULONG Length;
	BOOLEAN Initialized;
	ULONGLONG SsHandle;
	LIST_ENTRY64 InLoadOrderModuleList;
	LIST_ENTRY64 InMemoryOrderModuleList;
	LIST_ENTRY64 InInitializationOrderModuleList;
} PEB_LDR_DATA64, *PPEB_LDR_DATA64;

// Structure is cut down to ProcessHeap.
typedef struct _PEB64 {
	BOOLEAN InheritedAddressSpace;
	BOOLEAN ReadImageFileExecOptions;
	BOOLEAN BeingDebugged;
	BOOLEAN Spare;
	ULONGLONG Mutant;
	ULONGLONG ImageBaseAddress;
	ULONGLONG LoaderData;
	ULONGLONG ProcessParameters;
	ULONGLONG SubSystemData;
	ULONGLONG ProcessHeap;
} PEB64, *PPEB64;

typedef struct _UNICODE_STRING64 {
	USHORT Length;
	USHORT MaximumLength;
	ULONGLONG Buffer;
} UNICODE_STRING64;

typedef struct _LDR_DATA_TABLE_ENTRY64 {
	LIST_ENTRY64 InLoadOrderModuleList;
	LIST_ENTRY64 InMemoryOrderModuleList;
	LIST_ENTRY64 InInitializationOrderModuleList;
	ULONGLONG BaseAddress;
	ULONGLONG EntryPoint;
	DWORD64 SizeOfImage;
	UNICODE_STRING64 FullDllName;
	UNICODE_STRING64 BaseDllName;
	ULONG Flags;
	SHORT LoadCount;
	SHORT TlsIndex;
	LIST_ENTRY64 HashTableEntry;
	ULONGLONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY64, *PLDR_DATA_TABLE_ENTRY64;
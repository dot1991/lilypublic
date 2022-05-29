#pragma once
#include "common/kernel.h"
#include "common/obcallback.h"

#define ALTITUDE_BE L"363220"e

class EmptyWindow {
public:
	const HWND hWnd = CreateWindowExA(
		0, "Static"e, ""e, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		0, 0, GetModuleHandleA(0), 0);
	operator HWND() const { return hWnd; }
	~EmptyWindow() { DestroyWindow(hWnd); }
	void MessageLoop() const {
		MSG Msg;
		if (!PeekMessageA(&Msg, hWnd, 0, 0, PM_REMOVE))
			return;
		TranslateMessage(&Msg);
		DispatchMessageA(&Msg);
	}
};

class KernelLily : public Kernel {
public:
	DefBaseClass(tagWND_USER,
		MemberAtOffset(DWORD, dwExStyle, 0x18)
		MemberAtOffset(RECT, rcWindow, 0x58)
		MemberAtOffset(RECT, rcClient, 0x68)
		MemberAtOffset(WORD, FNID, 0x2A)
	);
	class PROP {};
	class THREADINFO {};

	class tagWND {
	private:
		constexpr static size_t Offset_OwningThread = 0x10;
		constexpr static size_t Offset_Prop = 0x90;
	public:
		PROP** GetPPProp(const KernelLily& kernel) const {
			return (PROP**)((uintptr_t)this + Offset_Prop);
		}
		PROP* GetPProp(const KernelLily& kernel) const {
			PROP* pProp = 0;
			if (!kernel.ReadProcessMemoryDBVM((uintptr_t)GetPPProp(kernel), &pProp, sizeof(pProp)))
				return 0;
			return pProp;
		}
		THREADINFO* GetThreadInfo(const KernelLily& kernel) const {
			THREADINFO* pThread = 0;
			if (!kernel.ReadProcessMemoryDBVM(((uintptr_t)this + Offset_OwningThread), &pThread, sizeof(pThread)))
				return 0;
			return pThread;
		}
		bool SetThreadInfo(const KernelLily& kernel, THREADINFO* pThread) {
			return kernel.WriteProcessMemoryDBVM(((uintptr_t)this + Offset_OwningThread), &pThread, sizeof(pThread));
		}
	};

	typedef struct _HEAD {
		HANDLE h;
		DWORD cLockObj;
	} HEAD, * PHEAD;

	typedef struct _THROBJHEAD {
		HEAD head;
		THREADINFO* pti;
	} THROBJHEAD, * PTHROBJHEAD;

	static_assert(sizeof(THROBJHEAD) == 0x18);

	typedef struct tagHIDDATA {
		THROBJHEAD head;
		tagWND* spwndTarget;
		RAWINPUT rid;   // raw input data, variable length
						// rid needs to be the last member in HIDDATA
	} HIDDATA, * PHIDDATA;

	KernelLily(const DBVM& dbvm) : Kernel(dbvm) {}

	const KernelFunction<PVOID(PVOID Process)> PsGetProcessWin32Process = {
		GetKernelProcAddressVerified("ntoskrnl.exe"e, "PsGetProcessWin32Process"e), *this };

	const KernelFunction<NTSTATUS(PVOID Process, PVOID Win32Process, PVOID PrevWin32Process)> PsSetProcessWin32Process = {
		GetKernelProcAddressVerified("ntoskrnl.exe"e, "PsSetProcessWin32Process"e), *this };

	const KernelFunction<PVOID(DWORD PoolType, SIZE_T NumberOfBytes)> ExAllocatePool = {
		GetKernelProcAddressVerified("ntoskrnl.exe"e, "ExAllocatePool"e), *this };

	const KernelFunction<NTSTATUS(EProcess* Process)> PsSuspendProcess = {
		GetKernelProcAddressVerified("ntoskrnl.exe"e, "PsSuspendProcess"e), *this };

	const KernelFunction<NTSTATUS(EProcess* Process)> PsResumeProcess = {
		GetKernelProcAddressVerified("ntoskrnl.exe"e, "PsResumeProcess"e), *this };

	const KernelFunction<tagWND* (HWND hWnd)> ValidateHwnd = {
		GetKernelProcAddressVerified("win32kbase.sys"e, "ValidateHwnd"e), *this };

	const KernelFunction<ATOM(PCWSTR AtomName)> UserFindAtom = {
		GetKernelProcAddressVerified("win32kbase.sys"e, "UserFindAtom"e), *this };

	const KernelFunction<HANDLE(PROP* pProp, ATOM nAtom, DWORD dwFlag)> RealGetProp = {
		GetKernelProcAddressVerified("win32kbase.sys"e, "RealGetProp"e), *this };

	const KernelFunction<BOOL(PROP* pProp, ATOM nAtom, DWORD dwFlag)> RealInternalRemoveProp = {
		GetKernelProcAddressVerified("win32kbase.sys"e, "RealInternalRemoveProp"e), *this };

	const KernelFunction<BOOL(PROP** pProp, ATOM nAtom, HANDLE hValue, DWORD dwFlag)> RealInternalSetProp = {
		GetKernelProcAddressVerified("win32kbase.sys"e, "RealInternalSetProp"e), *this };

	const SafeFunction<void(int, int)> EnterCrit =
		GetKernelProcAddressVerified("win32kbase.sys"e, "EnterCrit"e);

	const SafeFunction<void(void)> UserSessionSwitchLeaveCrit =
		GetKernelProcAddressVerified("win32kbase.sys"e, "UserSessionSwitchLeaveCrit"e);

	constexpr static BYTE TYPE_HIDDATA = 18;
	const SafeFunction<PVOID(THREADINFO* ptiOwner, void* pdeskSrc, BYTE bType, DWORD size)> _HMAllocObject =
		GetKernelProcAddressVerified("win32kbase.sys"e, "HMAllocObject"e);

	const KernelFunction<PVOID(THREADINFO* ptiOwner, void* pdeskSrc, BYTE bType, DWORD size)> HMAllocObject =
	{ [&](THREADINFO* ptiOwner, void* pdeskSrc, BYTE bType, DWORD size) -> PVOID {
		EnterCrit(1, 0);
		PVOID pResult = _HMAllocObject(ptiOwner, pdeskSrc, bType, size);
		UserSessionSwitchLeaveCrit();
		return pResult;
	}, *this };

	const SafeFunction<tagWND_USER* (HWND hWnd)> UserValidateHwnd = [&] {
		const uintptr_t ScanResult = PatternScan::Range((uintptr_t)IsChild, 0x30, "48 8B CA E8"e, ReadProcessMemoryWinAPI);
		verify(ScanResult);
		const auto pFunc = PatternScan::GetJumpAddress(ScanResult + 0x3, ReadProcessMemoryDBVM);
		verify(pFunc);
		return pFunc;
	}();

	const uintptr_t LpcRequestPort =
		GetKernelProcAddressVerified("ntoskrnl.exe"e, "LpcRequestPort"e);

	const SafeFunction<NTSTATUS(HWND hWnd, BOOL topmost)> NtUserDestroyDCompositionHwndTarget =
		GetUserProcAddressVerified("win32u.dll"e, "NtUserDestroyDCompositionHwndTarget"e);

private:
	const uintptr_t pPsProcessType = [&] {
		const uintptr_t ppPsProcessType = GetKernelProcAddressVerified("ntoskrnl.exe"e, "PsProcessType"e);
		ReadProcessMemoryDBVM(ppPsProcessType, (void*)&pPsProcessType, sizeof(pPsProcessType));
		verify(pPsProcessType);
		return pPsProcessType;
	}();

	uintptr_t GetCallbackEntryItemWithAltitude(const wchar_t* wAltitude) const {
		OBJECT_TYPE PsProcessType;
		if (!ReadProcessMemoryDBVM(pPsProcessType, &PsProcessType, sizeof(PsProcessType)))
			return 0;

		const uintptr_t pStartCallback = (uintptr_t)PsProcessType.CallbackList.Flink;
		uintptr_t pCallback = pStartCallback;
		do {
			CALLBACK_ENTRY_ITEM CallbackEntryItem;
			if (!ReadProcessMemoryDBVM(pCallback, &CallbackEntryItem, sizeof(CallbackEntryItem)))
				break;

			CALLBACK_ENTRY CallBackEntry;
			if (!ReadProcessMemoryDBVM((uintptr_t)CallbackEntryItem.CallbackEntry, &CallBackEntry, sizeof(CallBackEntry)))
				break;

			wchar_t wCallbackAltitude[0x100] = { 0 };
			if (!ReadProcessMemoryDBVM((uintptr_t)CallBackEntry.Altitude.Buffer, &wCallbackAltitude, CallBackEntry.Altitude.Length))
				break;

			if (wcscmp(wCallbackAltitude, wAltitude) == 0)
				return pCallback;

			pCallback = (uintptr_t)CallbackEntryItem.EntryItemList.Flink;
		} while (pCallback != pStartCallback);

		return 0;
	}

	PHIDDATA AllocateHidData(RAWINPUTHEADER Header, tagWND* pwnd) {
		if (!pwnd)
			return 0;

		THREADINFO* pti = pwnd->GetThreadInfo(*this);
		if (!pti)
			return 0;

		PHIDDATA pHidData = (PHIDDATA)HMAllocObject(pti, 0, TYPE_HIDDATA, Header.dwSize + FIELD_OFFSET(HIDDATA, rid.data));
		//Recalc the size of RAWINPUT structure.
		Header.dwSize += FIELD_OFFSET(RAWINPUT, data);
		if (!pHidData)
			return 0;

		tagWND* pWnd = 0;
		if (!WriteProcessMemoryDBVM((uintptr_t)&pHidData->spwndTarget, &pWnd, sizeof(pWnd)))
			return 0;

		//Lock(&pHidData->spwndTarget, pwnd);

		if (!WriteProcessMemoryDBVM((uintptr_t)&pHidData->rid.header, &Header, sizeof(Header)))
			return 0;

		return pHidData;
	}

	const HANDLE hHidMouseDevice = [&]()->HANDLE {
		HANDLE hDevice = 0;
		RAWINPUTDEVICELIST RawInputDeviceLists[0x100] = { 0 };
		UINT nDevice = 0x100;
		if (!GetRawInputDeviceList(RawInputDeviceLists, &nDevice, sizeof(RAWINPUTDEVICELIST)))
			return 0;

		for (auto RawInputDevice : RawInputDeviceLists) {
			if (!RawInputDevice.hDevice)
				break;
			if (RawInputDevice.dwType == RIM_TYPEMOUSE)
				return RawInputDevice.hDevice;
		}

		return 0;
	}();

	const PHIDDATA pHidData = [&] {
		verify(hHidMouseDevice);
		const RAWINPUTHEADER Header = {
			.dwType = RIM_TYPEMOUSE,
			.dwSize = sizeof(RAWMOUSE),
			.hDevice = hHidMouseDevice,
			.wParam = RIM_INPUT
		};
		const PHIDDATA pHidData = AllocateHidData(Header, ValidateHwnd(EmptyWindow()));
		verify(pHidData);
		return pHidData;
	}();

	const HANDLE hHidData = [&] {
		HANDLE h = 0;
		ReadProcessMemoryDBVM((uintptr_t)&pHidData->head.head.h, &h, sizeof(h));
		verify(h);
		return h;
	}();

public:
	bool PostRawMouseInput(HWND hWnd, RAWMOUSE RawMouse) {
		const bool IsValidMouseDevice = [&] {
			RID_DEVICE_INFO DeviceInfo{};
			UINT cbSize = sizeof(DeviceInfo);
			if (!GetRawInputDeviceInfoA(hHidMouseDevice, RIDI_DEVICEINFO, &DeviceInfo, &cbSize))
				return false;

			return DeviceInfo.dwType == RIM_TYPEMOUSE;
		}();
		verify(IsValidMouseDevice);

		if (!WriteProcessMemoryDBVM((uintptr_t)&pHidData->rid.data.mouse, &RawMouse, sizeof(RawMouse)))
			return false;

		DWORD_PTR Result;
		return SendMessageTimeoutA(hWnd, WM_INPUT, RIM_INPUT, (LPARAM)hHidData, SMTO_NORMAL, 1000, &Result);
	}

	ATOM UserFindAtomVerified(PCWSTR AtomName) const {
		const ATOM atom = UserFindAtom(AtomName);
		if (!atom)
			error(AtomName, L"Atom"e);
		return atom;
	}

	HANDLE KernelGetProp(HWND hWnd, ATOM nAtom, DWORD dwFlag = 1) const {
		tagWND* pWnd = ValidateHwnd(hWnd);
		if (!pWnd)
			return 0;

		PROP* pProp = pWnd->GetPProp(*this);
		if (!pProp)
			return 0;

		return RealGetProp(pProp, nAtom, dwFlag);
	}

	bool KernelRemoveProp(HWND hWnd, ATOM nAtom, DWORD dwFlag = 1) const {
		tagWND* pWnd = ValidateHwnd(hWnd);
		if (!pWnd)
			return 0;

		PROP* pProp = pWnd->GetPProp(*this);
		if (!pProp)
			return 0;

		return RealInternalRemoveProp(pProp, nAtom, dwFlag);
	}

	bool KernelSetProp(HWND hWnd, ATOM nAtom, HANDLE hValue, DWORD dwFlag = 5) const {
		tagWND* pWnd = ValidateHwnd(hWnd);
		if (!pWnd)
			return 0;

		return RealInternalSetProp(pWnd->GetPPProp(*this), nAtom, hValue, dwFlag);
	}

	bool PsSuspendProcessWrapper(DWORD Pid, auto f) const {
		if (Pid == GetCurrentProcessId())
			return false;

		EProcess* Process = GetEPROCESS(Pid);
		if (!Process)
			return false;

		if (PsSuspendProcess(Process) != 0)
			return false;

		Sleep(300);
		f();

		return PsResumeProcess(Process) == 0;
	}

	bool SetOwningThreadWrapper(HWND hWnd, auto f) const {
		const DWORD Pid = GetPIDFromHWND(hWnd);
		if (!Pid)
			return false;

		bool bSuccess = false;
		PsSuspendProcessWrapper(Pid, [&] {
			const EmptyWindow hWndFrom;
			tagWND* const pWndFrom = ValidateHwnd(hWndFrom);
			if (!pWndFrom)
				return;

			tagWND* const pWndTo = ValidateHwnd(hWnd);
			if (!pWndTo)
				return;

			THREADINFO* const pThreadFrom = pWndFrom->GetThreadInfo(*this);
			if (!pThreadFrom)
				return;

			THREADINFO* const pThreadTo = pWndTo->GetThreadInfo(*this);
			if (!pThreadTo)
				return;

			if (!pWndTo->SetThreadInfo(*this, pThreadFrom))
				return;

			f();

			if (!pWndTo->SetThreadInfo(*this, pThreadTo))
				return;

			bSuccess = true;
			});

		return bSuccess;
	}

	bool PsSetProcessWin32ProcessWrapper(HWND hWnd, auto f) const {
		DWORD dwPid;
		if (!GetWindowThreadProcessId(hWnd, &dwPid))
			return false;

		const PVOID CurrentProcess = (PVOID)GetEPROCESS(GetCurrentProcessId());
		if (!CurrentProcess)
			return false;

		const PVOID ProcessHWND = (PVOID)GetEPROCESS(dwPid);
		if (!ProcessHWND)
			return false;

		const PVOID Win32ProcessCurrent = PsGetProcessWin32Process(CurrentProcess);
		const PVOID Win32ProcessHWND = PsGetProcessWin32Process(ProcessHWND);
		PsSetProcessWin32Process(CurrentProcess, 0, Win32ProcessCurrent);
		PsSetProcessWin32Process(CurrentProcess, Win32ProcessHWND, 0);
		f();
		PsSetProcessWin32Process(CurrentProcess, 0, Win32ProcessHWND);
		PsSetProcessWin32Process(CurrentProcess, Win32ProcessCurrent, 0);

		const bool bWin32ProcessRollbacked = (PsGetProcessWin32Process(CurrentProcess) == Win32ProcessCurrent);
		verify(bWin32ProcessRollbacked);
		return true;
	}
};
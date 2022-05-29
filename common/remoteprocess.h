#pragma once
#include "common/kernel.h"
#include "shellcode.h"

class RemoteProcess {
private:
	const std::shared_ptr<const Kernel> pKernel;
	const HANDLE hProcess;
	const DWORD dwPid;
	void* RemoteEntryPoint;
	CR3 cr3;
public:
	void SuspendResumeWrapper(auto f) const {
		EnumAllThreads([&](THREADENTRY32 te32) {
			if (te32.th32OwnerProcessID != dwPid)
				return true;

			const HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
			if (!hThread)
				return true;

			SuspendThread(hThread);
			CONTEXT Context = { .ContextFlags = CONTEXT_ALL };
			GetThreadContext(hThread, &Context);
			CloseHandle(hThread);
			return true;
			});

		f();

		EnumAllThreads([&](THREADENTRY32 te32) {
			if (te32.th32OwnerProcessID != dwPid)
				return true;

			const HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
			if (!hThread)
				return true;

			ResumeThread(hThread);
			CloseHandle(hThread);
			return true;
			});
	}

	bool VirtualProtectWrapper(void* pRemoteAddress, size_t Size, auto f) const {
		DWORD dwOldProtect;
		if (!RemoteVirtualProtect(pRemoteAddress, Size, PAGE_EXECUTE_READWRITE, &dwOldProtect))
			return false;

		f();

		if (!RemoteVirtualProtect(pRemoteAddress, Size, dwOldProtect, &dwOldProtect))
			return false;
		return true;
	}

	bool TriggerCOW(void* pRemoteAddress, size_t Size) const {
		std::vector<uint8_t> OriginalBytes(Size);
		bool bResult = false;
		if (!VirtualProtectWrapper(pRemoteAddress, Size, [&] {
			if (!RemoteReadProcessMemory(pRemoteAddress, OriginalBytes.data(), Size, 0))
				return;
			if (!RemoteWriteProcessMemory(pRemoteAddress, OriginalBytes.data(), Size, 0))
				return;
			bResult = true;
			})) return false;
		return bResult;
	}

	bool WriteProcessMemoryWinAPIWrapper(void* pRemoteAddress, const void* pBuffer, size_t Size, auto f) const {
		std::vector<uint8_t> OriginalBytes(Size);
		if (!RemoteReadProcessMemory(pRemoteAddress, OriginalBytes.data(), Size, 0))
			return false;

		if (!VirtualProtectWrapper(pRemoteAddress, Size, [&] {
			if (!RemoteWriteProcessMemory(pRemoteAddress, pBuffer, Size, 0))
				return false;
			f();
			if (!RemoteWriteProcessMemory(pRemoteAddress, OriginalBytes.data(), Size, 0))
				return false;
			return true;
			})) return false;

		return true;
	}

	bool WriteProcessMemoryCloakWrapper(void* pRemoteAddress, const void* pBuffer, size_t Size, auto f) const {
		verify(cr3);
		//if (!TriggerCOW(pRemoteAddress, Size))
		//	return false;

		std::vector<uint8_t> OriginalBytes(Size);
		if (!RemoteReadProcessMemory(pRemoteAddress, OriginalBytes.data(), Size, 0))
			return false;

		if (!pKernel->dbvm.WPMCloak((uintptr_t)pRemoteAddress, pBuffer, Size, cr3))
			return false;
		f();
		if (!pKernel->dbvm.WPMCloak((uintptr_t)pRemoteAddress, OriginalBytes.data(), Size, cr3))
			return false;

		return true;
	}

	bool WriteProcessMemoryWrapper(void* pRemoteAddress, const void* pBuffer, size_t Size, auto f) const {
		if (cr3)
			return WriteProcessMemoryCloakWrapper(pRemoteAddress, pBuffer, Size, f);
		return WriteProcessMemoryWinAPIWrapper(pRemoteAddress, pBuffer, Size, f);
	}

	HANDLE CreateRemoteThreadParams(auto pStartAddress, auto Rax, auto Rcx, auto Rdx, auto R8, auto R9, bool bExitProcess) const {
		const HANDLE hThread = RemoteCreateRemoteThread(0, 0, (LPTHREAD_START_ROUTINE)RemoteEntryPoint, 0, CREATE_SUSPENDED, 0);
		if (!hThread)
			return 0;

		const bool bSuccess = [&] {
			CONTEXT Context = { .ContextFlags = CONTEXT_ALL };
			if (!GetThreadContext(hThread, &Context))
				return false;

			Context.Rax = (uintptr_t)Rax;
			Context.Rcx = (uintptr_t)Rcx;
			Context.Rdx = (uintptr_t)Rdx;
			Context.R8 = (uintptr_t)R8;
			Context.R9 = (uintptr_t)R9;
			Context.Rip = (uintptr_t)pStartAddress;

			uintptr_t pExitFunc = 0;
			if (!bExitProcess)
				pExitFunc = (uintptr_t)ExitThread;

			if (!RemoteWriteProcessMemory((void*)Context.Rsp, &pExitFunc, sizeof(pExitFunc), 0))
				return false;

			if (!SetThreadContext(hThread, &Context))
				return false;

			if (ResumeThread(hThread) == -1)
				return false;

			return true;
		}();

		if (!bSuccess) {
			TerminateThread(hThread, 0);
			CloseHandle(hThread);
			return 0;
		}
		
		return hThread;
	}

	BOOL RemoteVirtualProtect(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect) const {
		return VirtualProtectEx(hProcess, lpAddress, dwSize, flNewProtect, lpflOldProtect);
	}

	LPVOID RemoteVirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect) const {
		return VirtualAllocEx(hProcess, lpAddress, dwSize, flAllocationType, flProtect);
	}

	BOOL RemoteVirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType) const {
		return VirtualFreeEx(hProcess, lpAddress, dwSize, dwFreeType);
	}

	BOOL RemoteWriteProcessMemory(LPVOID lpBaseAddress, LPCVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesWritten) const {
		return WriteProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesWritten);
	}

	BOOL RemoteReadProcessMemory(LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead) const {
		return ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead);
	}

	SIZE_T RemoteVirtualQuery(LPCVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength) const {
		return VirtualQueryEx(hProcess, lpAddress, lpBuffer, dwLength);
	}

	HANDLE RemoteCreateRemoteThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress,
		LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId) const {
		return CreateRemoteThread(hProcess, lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
	}

	BOOL RemoteTerminateProcess(UINT uExitCode) const {
		return TerminateProcess(hProcess, uExitCode);
	}

	BOOL RemoteGetModuleInformation(HMODULE hModule, LPMODULEINFO lpmodinfo, DWORD cb) const {
		return GetModuleInformation(hProcess, hModule, lpmodinfo, cb);
	}

	DWORD RemoteGetModuleFileNameExA(HMODULE hModule, LPSTR lpFilename, DWORD nSize) const {
		return GetModuleFileNameExA(hProcess, hModule, lpFilename, nSize);
	}

	RemoteProcess(DWORD dwPid, const std::shared_ptr<const Kernel> pKernel) :
		hProcess(OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid)), pKernel(pKernel), dwPid(dwPid),
		cr3(pKernel ? pKernel->GetKernelCR3(dwPid) : CR3()) {
		const bool bSuccess = [&] {
			if (!hProcess)
				return false;
			HMODULE hModule;
			DWORD dwSize;
			if (!EnumProcessModules(hProcess, &hModule, sizeof(hModule), &dwSize))
				return false;
			MODULEINFO ModuleInfo;
			if (!RemoteGetModuleInformation(hModule, &ModuleInfo, sizeof(ModuleInfo)))
				return false;
			RemoteEntryPoint = ModuleInfo.EntryPoint;
			return true;
		}();
		verify(bSuccess);
	}

	~RemoteProcess() {
		if (pKernel)
			pKernel->dbvm.CloakReset();
		CloseHandle(hProcess);
	}

	bool RemoveNXBit(void* pRemoteAddress, size_t Size) const {
		verify(cr3);

		bool bSuccess = false;

		SuspendResumeWrapper([&] {
			DWORD dwOldProtect;
			if (!RemoteVirtualProtect(pRemoteAddress, Size, PAGE_EXECUTE_READWRITE, &dwOldProtect) ||
				!RemoteVirtualProtect(pRemoteAddress, Size, PAGE_READWRITE, &dwOldProtect)) {
				return;
			}

			for (size_t i = 0; i < Size; i += 0x1000) {
				uintptr_t VirtualAddress = (uintptr_t)pRemoteAddress + i;

				uint8_t byte;
				if (!RemoteReadProcessMemory((void*)VirtualAddress, &byte, sizeof(byte), 0))
					return;

				PhysicalAddress PTEAddress = pKernel->dbvm.GetPTEAddress(VirtualAddress, cr3);
				if (!PTEAddress)
					return;

				PTE pte;
				if (!pKernel->dbvm.ReadPhysicalMemory(PTEAddress, &pte, sizeof(pte)))
					return;
				pte.ExecuteDisable = 0;
				if (!pKernel->dbvm.WritePhysicalMemory(PTEAddress, &pte, sizeof(pte)))
					return;
			}

			bSuccess = true;
			});

		return bSuccess;
	}

	bool BypassCFG() const {
		HMODULE hKernel32 = GetModuleHandleA("kernel32.dll"e);
		if (!hKernel32)
			return false;

		PIMAGE_DOS_HEADER dosHd = (PIMAGE_DOS_HEADER)hKernel32;
		PIMAGE_NT_HEADERS ntHd = (PIMAGE_NT_HEADERS)((uintptr_t)hKernel32 + dosHd->e_lfanew);

		if (!ntHd->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].Size)
			return true;

		PIMAGE_LOAD_CONFIG_DIRECTORY loadconfig = (PIMAGE_LOAD_CONFIG_DIRECTORY)((uintptr_t)hKernel32 +
			ntHd->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress);

		void* pFunc = 0;
		ShellCode_Ret shellcode_ret;
		RemoteReadProcessMemory((void*)loadconfig->GuardCFCheckFunctionPointer, &pFunc, sizeof(void*), 0);
		if (pFunc && !RemoteWriteProcessMemory(pFunc, &shellcode_ret, sizeof(shellcode_ret), 0))
			return false;

		pFunc = 0;
		ShellCode_JmpRax shellcode_jmprax;
		RemoteReadProcessMemory((void*)loadconfig->GuardCFDispatchFunctionPointer, &pFunc, sizeof(void*), 0);
		if (pFunc && !RemoteWriteProcessMemory(pFunc, &shellcode_jmprax, sizeof(shellcode_jmprax), 0))
			return false;

		return true;
	}

	bool IsPlatformMatched() const {
		BOOL bWow64 = TRUE;
		IsWow64Process(hProcess, &bWow64);

		SYSTEM_INFO SystemInfo;
		GetNativeSystemInfo(&SystemInfo);

#ifdef _WIN64
		return SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 && bWow64 == FALSE;
#else
		if (SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
			return TRUE;

		return SystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 && bWow64 == TRUE;
#endif
	}

	bool VirtualAllocWrapper(const void* pBuffer, size_t Size, auto f) const {
		void* pAllocBase = RemoteVirtualAlloc(0, Size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!pAllocBase)
			return false;

		if (!RemoteWriteProcessMemory(pAllocBase, pBuffer, Size, 0)) {
			RemoteVirtualFree(pAllocBase, 0, MEM_RELEASE);
			return false;
		}

		f((const void*)pAllocBase);
		return RemoteVirtualFree(pAllocBase, 0, MEM_RELEASE);
	}

	template<class R, class A1, class A2, class A3, class A4>
	bool RemoteCall(void* pFunc, R pResult, A1 a1, A2 a2, A3 a3, A4 a4, bool bWaitUntilExit, bool bExitProcess) const {
		bool bResult1 = false;
		bool bResult2 = false;

		if (bWaitUntilExit) {
			ShellCode_CallRaxLoop shellcode;
			bResult1 = WriteProcessMemoryWrapper(RemoteEntryPoint, &shellcode, sizeof(shellcode), [&] {
				const HANDLE hThread = CreateRemoteThreadParams(RemoteEntryPoint, pFunc, a1, a2, a3, a4, bExitProcess);
				if (!hThread)
					return;
				bResult2 = [&] {
					Sleep(1);
					for (unsigned i = 0; i < 10; i++, Sleep(100)) {
						CONTEXT Context = { .ContextFlags = CONTEXT_ALL };
						if (!GetThreadContext(hThread, &Context))
							return false;
						if (Context.Rip != (uintptr_t)RemoteEntryPoint + shellcode.OffsetLoopOpcode)
							continue;
						if constexpr (std::is_pointer_v<R>) if (pResult)
							*pResult = (std::remove_pointer_t<R>)Context.Rax;
						return TerminateThread(hThread, 0) == TRUE;
					}
					return false;
				}();
				CloseHandle(hThread);
				});
		}
		else {
			ShellCode_JmpRax shellcode;
			bResult1 = WriteProcessMemoryWrapper(RemoteEntryPoint, &shellcode, sizeof(shellcode), [&] {
				const HANDLE hThread = CreateRemoteThreadParams(RemoteEntryPoint, pFunc, a1, a2, a3, a4, bExitProcess);
				if (!hThread)
					return;
				Sleep(1);
				bResult2 = true;
				});
		}

		return bResult1 && bResult2;
	}
};
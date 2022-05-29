#include "injectorUI.h"

#include <memory>
#include <tlhelp32.h>
#include "common/kernel.h"
#include "common/dbvm.h"
#include "injector.h"

#define FILE_READ_ACCESS          ( 0x0001 )    // file & pipe
#define FILE_WRITE_ACCESS         ( 0x0002 )    // file & pipe
#define METHOD_BUFFERED                 0
#define FILE_DEVICE_UNKNOWN             0x00000022
#define IOCTL_UNKNOWN_BASE					FILE_DEVICE_UNKNOWN
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)
#define IOCTL_CE_LAUNCHDBVM						CTL_CODE(IOCTL_UNKNOWN_BASE, 0x083a, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_CE_READMSR						CTL_CODE(IOCTL_UNKNOWN_BASE, 0x083f, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_CE_ALLOCATE_MEMORY_FOR_DBVM		CTL_CODE(IOCTL_UNKNOWN_BASE, 0x0862, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_CE_VMXCONFIG						CTL_CODE(IOCTL_UNKNOWN_BASE, 0x082d, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

bool VMXConfig(HANDLE hDevice, const DBVM& dbvm) {
#pragma pack(push, 1)
	struct
	{
		uint32_t Virtualization_Enabled;
		uint64_t Password1;
		uint32_t Password2;
		uint64_t Password3;
	} input;
#pragma pack(pop, 1)

	if (!dbvm.GetVersion())
		return false;

	input.Virtualization_Enabled = 1;
	dbvm.GetPassword(input.Password1, input.Password2, input.Password3);

	DWORD BytesReturned;
	return DeviceIoControl(hDevice, IOCTL_CE_VMXCONFIG, &input, sizeof(input), 0, 0, &BytesReturned, 0);
}

void LaunchDBVM(HANDLE hDevice, std::wstring wPathImg) {
	struct intput
	{
		const wchar_t* dbvmimgpath;
		uint32_t cpuid;
	}input;

	input.cpuid = 0xFFFFFFFF;
	input.dbvmimgpath = wPathImg.c_str();

	DWORD BytesReturned;
	DeviceIoControl(hDevice, IOCTL_CE_LAUNCHDBVM, &input, sizeof(input), 0, 0, &BytesReturned, NULL);
}

uint64_t ReadMSR(HANDLE hDevice, uint32_t MSR) {
	uint64_t Result = 0;
	DWORD BytesReturned;
	DeviceIoControl(hDevice, IOCTL_CE_READMSR, &MSR, sizeof(MSR), &Result, sizeof(Result), &BytesReturned, 0);
	return Result;
}

bool AddMemoryDBVM(HANDLE hDevice, uint64_t PageCount) {
	DWORD BytesReturned;
	return DeviceIoControl(hDevice, IOCTL_CE_ALLOCATE_MEMORY_FOR_DBVM, &PageCount, sizeof(PageCount), 0, 0, &BytesReturned, 0);
}

DWORD GetCPUCount() {
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
}

bool IsFileExist(const char* szPath) {
	FILE* in = fopen(szPath, "r"e);
	if (!in)
		return false;
	fclose(in);
	return true;
}

void StopDriver(const char* szServiceName) {
	SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	SC_HANDLE schService = OpenServiceA(schSCManager, szServiceName, SERVICE_ALL_ACCESS);
	if (!schService)
		return;

	for (unsigned i = 0; i < 5; i++) {
		SERVICE_STATUS serviceStatus;
		if (ControlService(schService, SERVICE_CONTROL_STOP, &serviceStatus))
			break;

		if (GetLastError() != ERROR_DEPENDENT_SERVICES_RUNNING)
			break;

		Sleep(1);
	}

	DeleteService(schService);
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

bool StartDriver(const char* szServiceName, const char* szFilePath) {
	StopDriver(szServiceName);

	SC_HANDLE schSCManager = OpenSCManagerA(0, 0, SC_MANAGER_ALL_ACCESS);
	if (!schSCManager)
		return false;

	SC_HANDLE schService = CreateServiceA(schSCManager, // SCManager database
		szServiceName,         // name of service
		szServiceName,         // name to display
		SERVICE_ALL_ACCESS,    // desired access
		SERVICE_KERNEL_DRIVER, // service type
		SERVICE_DEMAND_START,  // start type
		SERVICE_ERROR_NORMAL,  // error control type
		szFilePath,            // service's binary
		0,					   // no load ordering group
		0,					    // no tag identifier
		0,					    // no dependencies
		0,					   // LocalSystem account
		0					   // no password
	);
	if (!schService) {
		CloseServiceHandle(schSCManager);
		return false;
	}

	bool bSuccess = StartServiceA(schService, 0, 0);
	DWORD dwError = GetLastError();
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	return bSuccess || (dwError == ERROR_SERVICE_ALREADY_RUNNING);
}

DWORD GetPID(char* szProcessName, bool bCreateProcess) {
	if (!bCreateProcess)
		return GetPIDByProcessName(szProcessName);

	STARTUPINFO StartupInfo = { .cb = sizeof(StartupInfo) };
	PROCESS_INFORMATION ProcessInfo = {};
	if (!CreateProcessA(0, szProcessName, 0, 0, 0, CREATE_SUSPENDED, 0, 0, &StartupInfo, &ProcessInfo))
		return 0;

	CONTEXT Context = { .ContextFlags = CONTEXT_ALL };
	GetThreadContext(ProcessInfo.hThread, &Context);
	Context.Rcx = -1;
	Context.Rip = (uintptr_t)Sleep;
	SetThreadContext(ProcessInfo.hThread, &Context);
	ResumeThread(ProcessInfo.hThread);

	bool IsLoaded = [&] {
		for (unsigned i = 0; i < 100; i++) {
			Sleep(10);
			DWORD dwSize;
			HMODULE hModule;
			if (EnumProcessModules(ProcessInfo.hProcess, &hModule, sizeof(hModule), &dwSize))
				return true;
		}
		TerminateProcess(ProcessInfo.hProcess, 0);
		return false;
	}();

	CloseHandle(ProcessInfo.hThread);
	CloseHandle(ProcessInfo.hProcess);
	return IsLoaded ? ProcessInfo.dwProcessId : 0;
}

void InjectorUI::OnButtonDBVM() {
	std::string strMsg;
	HANDLE hDevice = INVALID_HANDLE_VALUE;

	bool bSuccess = [&] {
		bool IsDBVMAlreadyLoaded = [&] {
			if (dbvm.GetVersion())
				return true;

			dbvm.SetPassword(default_password1, default_password2, default_password3);
			if (dbvm.GetVersion())
				return true;

			return false;
		}();

		if (IsDBVMAlreadyLoaded) {
			strMsg = (std::string)"dbvm already loaded. "e;
			strMsg += std::to_string(dbvm.GetMemory() / 0x1000);
			strMsg += (const char*)" pages free"e;
			return true;
		}

		if (!DBVM::IsDBVMCapable()) {
			strMsg = (std::string)"Your system DOES NOT support DBVM."e;
			return false;
		}

		char szCurrentDir[0x100];
		GetCurrentDirectoryA(sizeof(szCurrentDir), szCurrentDir);

		std::string strPathSys = szCurrentDir;
		strPathSys += (const char*)"\\"e;
		strPathSys += szSysFileName;

		std::string strPathImg = szCurrentDir;
		strPathImg += (const char*)"\\vmdisk.img"e;

		if (!IsFileExist(strPathSys.c_str())) {
			strMsg = szSysFileName;
			strMsg += (const char*)" does not exist"e;
			return false;
		}

		if (!IsFileExist(strPathImg.c_str())) {
			strMsg = (std::string)"vmdisk.img does not exist."e;
			return false;
		}

		std::string strSubKey = "SYSTEM\\CurrentControlSet\\Services\\"e;
		strSubKey += szServiceName;

		HKEY hKey;
		if (RegCreateKeyA(HKEY_LOCAL_MACHINE, strSubKey.c_str(), &hKey) || RegOpenKeyA(HKEY_LOCAL_MACHINE, strSubKey.c_str(), &hKey)) {
			strMsg = (std::string)"Cannot open registry."e;
			return false;
		}

		std::string strRegValue;
		strRegValue = (std::string)"\\Device\\"e + szServiceName;
		RegSetValueExA(hKey, "A"e, 0, REG_SZ, (const BYTE*)strRegValue.c_str(), DWORD(strRegValue.size() + 1));
		strRegValue = (std::string)"\\DosDevices\\"e + szServiceName;
		RegSetValueExA(hKey, "B"e, 0, REG_SZ, (const BYTE*)strRegValue.c_str(), DWORD(strRegValue.size() + 1));
		strRegValue = (std::string)"\\BaseNamedObjects\\"e + szProcessEventName;
		RegSetValueExA(hKey, "C"e, 0, REG_SZ, (const BYTE*)strRegValue.c_str(), DWORD(strRegValue.size() + 1));
		strRegValue = (std::string)"\\BaseNamedObjects\\"e + szThreadEventName;
		RegSetValueExA(hKey, "D"e, 0, REG_SZ, (const BYTE*)strRegValue.c_str(), DWORD(strRegValue.size() + 1));
		RegCloseKey(hKey);

		if (!StartDriver(szServiceName, strPathSys.c_str())) {
			strMsg = (std::string)"Could not launch DBVM: StartService Failed."e;
			return false;
		}

		hDevice = CreateFileA(
			((std::string)"\\\\.\\"e + szServiceName).c_str(),
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

		if (hDevice == INVALID_HANDLE_VALUE) {
			strMsg = (std::string)"Could not launch DBVM: INVALID_HANDLE_VALUE."e;
			return false;
		}

		if (dbvm.IsAMD()) {
			uint64_t msr = ReadMSR(hDevice, 0xC0010114);
			if ((msr & (1 << 3)) && (msr & (1 << 4))) {
				strMsg = (std::string)"Could not launch DBVM: The AMD-v feature has been disabled in your BIOS."e;
				return false;
			}
		}
		else {
			uint64_t msr = ReadMSR(hDevice, 0x3A);
			if ((msr & 1) && !(msr & (1 << 2))) {
				strMsg = (std::string)"Could not launch DBVM: The Intel-VT feature has been disabled in your BIOS."e;
				return false;
			}
		}

		LaunchDBVM(hDevice, (std::wstring)L"\\??\\"e + std::wstring(strPathImg.begin(), strPathImg.end()));

		dbvm.SetDefaultPassword();
		if (!dbvm.GetVersion()) {
			strMsg = (std::string)"Could not launch DBVM: DeviceIoControl Failed."e;
			return false;
		}

		dbvm.ChangePassword(default_password1, default_password2, default_password3);
		SetPasswordFromParam();

		bool IsHided = false;
		if (VMXConfig(hDevice, dbvm) && AddMemoryDBVM(hDevice, (uint64_t)GetCPUCount() * 0x30)) {
			dbvm.HideDBVM();
			IsHided = true;
		}

		strMsg = (std::string)"dbvm loaded. "e;
		strMsg += std::to_string(dbvm.GetMemory() / 0x1000);
		strMsg += (const char*)" pages free"e;
		if (IsHided)
			strMsg += (const char*)"\ndbvm hide completed."e;

		return true;
	}();

	CloseHandle(hDevice);
	StopDriver(szServiceName);
	MessageBoxA(hWnd, strMsg.c_str(), ""e, bSuccess ? 0 : MB_ICONERROR);
}

bool InjectorUI::SetPasswordFromParam() {
	if (strlen(szParam) < 20)
		return false;

	uint64_t password1 = *(uint64_t*)(szParam + 0) ^ 0xda2355698be6166c;
	uint32_t password2 = *(uint32_t*)(szParam + 8) ^ 0x6765fa70;
	uint64_t password3 = *(uint64_t*)(szParam + 12) ^ 0xe21cb5155c065962;

	dbvm.SetPassword(password1, password2, password3);
	if (dbvm.GetVersion())
		return true;

	dbvm.SetPassword(default_password1, default_password2, default_password3);
	if (ExceptionHandler::TryExcept([&] { dbvm.ChangePassword(password1, password2, password3); }))
		return true;

	return false;
}

void InjectorUI::OnButtonSetPassword() {
	if (SetPasswordFromParam()) MessageBoxA(hWnd, "Success", ""e, 0);
	else MessageBoxA(hWnd, "Failed", ""e, MB_ICONERROR);
}

void InjectorUI::OnButtonInject() {
	std::string strMsg;

	bInjected = [&] {
		FILE* in = fopen(szImageName, "r"e);
		if (!in) {
			strMsg = (std::string)"Image not exist."e;
			return false;
		}
		fclose(in);

		if (InjectionType == EInjectionType::NxBitSwap && !dbvm.GetVersion()) {
			strMsg = (std::string)"DBVM is not loaded."e;
			return false;
		}

		const DWORD dwPid = GetPID(szProcessName, bCreateProcess);
		if (!dwPid) {
			strMsg = (std::string)"No target process"e;
			return false;
		}

		const std::shared_ptr<const Kernel> pKernel(dbvm.GetVersion() ? new Kernel(dbvm) : 0);
		const RemoteProcess remoteprocess(dwPid, pKernel);
		Injector injector(remoteprocess);

		bool Result = injector.MapRemoteModuleAFromFileName(szImageName, true, szParam, InjectionType, szIntoDLL);
		strMsg = injector.GetErrorMsg();

		if (!Result)
			remoteprocess.RemoteTerminateProcess(0);

		return Result;
	}();

	if (!bInjected) {
		MessageBoxA(hWnd, strMsg.c_str(), ""e, MB_ICONERROR);
		return;
	}
}
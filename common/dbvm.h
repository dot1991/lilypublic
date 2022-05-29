#pragma once
#include <Windows.h>
#include <intrin.h>
#include "physicalmemory.h"
#include "common/ida_defs.h"
#include "common/exception.h"
#include "function_ref.hpp"

extern "C" uintptr_t vmcall_intel(uint64_t Password3, uint64_t Password1, void* pVMCallInfo);
extern "C" uintptr_t vmcall_amd(uint64_t Password3, uint64_t Password1, void* pVMCallInfo);

#pragma pack(push, 1)

struct ChangeRegOnBPInfo {
	struct {
		bool changeRAX : 1;			//0
		bool changeRBX : 1;			//1
		bool changeRCX : 1;			//2
		bool changeRDX : 1;        //3
		bool changeRSI : 1;        //4
		bool changeRDI : 1;        //5
		bool changeRBP : 1;        //6
		bool changeRSP : 1;        //7
		bool changeRIP : 1;        //8
		bool changeR8 : 1;        //9
		bool changeR9 : 1;        //10
		bool changeR10 : 1;        //11
		bool changeR11 : 1;        //12
		bool changeR12 : 1;        //13
		bool changeR13 : 1;        //14
		bool changeR14 : 1;        //15
		bool changeR15 : 1;        //16
		bool changeCF : 1;         //17
		bool changePF : 1;         //18
		bool changeAF : 1;         //19
		bool changeZF : 1;         //20
		bool changeSF : 1;         //21
		bool changeOF : 1;         //22
		bool newCF : 1;            //23
		bool newPF : 1;            //24
		bool newAF : 1;            //25
		bool newZF : 1;            //26
		bool newSF : 1;            //27
		bool newOF : 1;            //28
		bool reserved : 3;         //29,30,31
	};
	struct {
		bool changeXMM0_0 : 1, changeXMM0_1 : 1, changeXMM0_2 : 1, changeXMM0_3 : 1;
		bool changeXMM1_0 : 1, changeXMM1_1 : 1, changeXMM1_2 : 1, changeXMM1_3 : 1;
		bool changeXMM2_0 : 1, changeXMM2_1 : 1, changeXMM2_2 : 1, changeXMM2_3 : 1;
		bool changeXMM3_0 : 1, changeXMM3_1 : 1, changeXMM3_2 : 1, changeXMM3_3 : 1;
		bool changeXMM4_0 : 1, changeXMM4_1 : 1, changeXMM4_2 : 1, changeXMM4_3 : 1;
		bool changeXMM5_0 : 1, changeXMM5_1 : 1, changeXMM5_2 : 1, changeXMM5_3 : 1;
		bool changeXMM6_0 : 1, changeXMM6_1 : 1, changeXMM6_2 : 1, changeXMM6_3 : 1;
		bool changeXMM7_0 : 1, changeXMM7_1 : 1, changeXMM7_2 : 1, changeXMM7_3 : 1;
		bool changeXMM8_0 : 1, changeXMM8_1 : 1, changeXMM8_2 : 1, changeXMM8_3 : 1;
		bool changeXMM9_0 : 1, changeXMM9_1 : 1, changeXMM9_2 : 1, changeXMM9_3 : 1;
		bool changeXMM10_0 : 1, changeXMM10_1 : 1, changeXMM10_2 : 1, changeXMM10_3 : 1;
		bool changeXMM11_0 : 1, changeXMM11_1 : 1, changeXMM11_2 : 1, changeXMM11_3 : 1;
		bool changeXMM12_0 : 1, changeXMM12_1 : 1, changeXMM12_2 : 1, changeXMM12_3 : 1;
		bool changeXMM13_0 : 1, changeXMM13_1 : 1, changeXMM13_2 : 1, changeXMM13_3 : 1;
		bool changeXMM14_0 : 1, changeXMM14_1 : 1, changeXMM14_2 : 1, changeXMM14_3 : 1;
		bool changeXMM15_0 : 1, changeXMM15_1 : 1, changeXMM15_2 : 1, changeXMM15_3 : 1;
	};
	uint64_t changeFP; //just one bit, each bit is a fpu field
	uint64_t newRAX;
	uint64_t newRBX;
	uint64_t newRCX;
	uint64_t newRDX;
	uint64_t newRSI;
	uint64_t newRDI;
	uint64_t newRBP;
	uint64_t newRSP;
	uint64_t newRIP;
	uint64_t newR8;
	uint64_t newR9;
	uint64_t newR10;
	uint64_t newR11;
	uint64_t newR12;
	uint64_t newR13;
	uint64_t newR14;
	uint64_t newR15;
	uint64_t newFP0;
	uint64_t newFP0_H;
	uint64_t newFP1;
	uint64_t newFP1_H;
	uint64_t newFP2;
	uint64_t newFP2_H;
	uint64_t newFP3;
	uint64_t newFP3_H;
	uint64_t newFP4;
	uint64_t newFP4_H;
	uint64_t newFP5;
	uint64_t newFP5_H;
	uint64_t newFP6;
	uint64_t newFP6_H;
	uint64_t newFP7;
	uint64_t newFP7_H;
	union {
		struct { uint64_t uint64_0, uint64_1; };
		struct { double Double_0, Double_1; };
		struct { uint32_t uint32_0, uint32_1, uint32_2, uint32_3; };
		struct { float Float_0, Float_1, Float_2, Float_3; };
	} XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6, XMM7, XMM8, XMM9, XMM10, XMM11, XMM12, XMM13, XMM14, XMM15;
};

class DBVM {
public:
	constexpr static uint64_t default_password1 = 0x76543210;
	constexpr static uint32_t default_password2 = 0xfedcba98;
	constexpr static uint64_t default_password3 = 0x90909090;
private:
	constexpr static uint16_t KernelCS = 0x10;
	bool bIntel = true;
	uint64_t current_password1 = default_password1;
	uint32_t current_password2 = default_password2;
	uint64_t current_password3 = default_password3;
	//////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////
	constexpr static uint32_t VMCALL_GETVERSION = 0;
	constexpr static uint32_t VMCALL_CHANGEPASSWORD = 1;
	constexpr static uint32_t VMCALL_READ_PHYSICAL_MEMORY = 3;
	constexpr static uint32_t VMCALL_WRITE_PHYSICAL_MEMORY = 4;
	constexpr static uint32_t VMCALL_REDIRECTINT1 = 9;
	constexpr static uint32_t VMCALL_INT1REDIRECTED = 10;
	constexpr static uint32_t VMCALL_CHANGESELECTORS = 12;
	constexpr static uint32_t VMCALL_BLOCK_INTERRUPTS = 13;
	constexpr static uint32_t VMCALL_RESTORE_INTERRUPTS = 14;
	constexpr static uint32_t VMCALL_REGISTER_CR3_EDIT_CALLBACK = 16;
	constexpr static uint32_t VMCALL_RETURN_FROM_CR3_EDIT_CALLBACK = 17;
	constexpr static uint32_t VMCALL_GETCR0 = 18;
	constexpr static uint32_t VMCALL_GETCR3 = 19;
	constexpr static uint32_t VMCALL_GETCR4 = 20;
	constexpr static uint32_t VMCALL_RAISEPRIVILEGE = 21;
	constexpr static uint32_t VMCALL_REDIRECTINT14 = 22;
	constexpr static uint32_t VMCALL_INT14REDIRECTED = 23;
	constexpr static uint32_t VMCALL_REDIRECTINT3 = 24;
	constexpr static uint32_t VMCALL_INT3REDIRECTED = 25;
	//dbvm v6+
	constexpr static uint32_t VMCALL_READMSR = 26;
	constexpr static uint32_t VMCALL_WRITEMSR = 27;
	constexpr static uint32_t VMCALL_ULTIMAP = 28;
	constexpr static uint32_t VMCALL_ULTIMAP_DISABLE = 29;
	//dbvm v7
	constexpr static uint32_t VMCALL_SWITCH_TO_KERNELMODE = 30;
	constexpr static uint32_t VMCALL_DISABLE_DATAPAGEFAULTS = 31;
	constexpr static uint32_t VMCALL_ENABLE_DATAPAGEFAULTS = 32;
	constexpr static uint32_t VMCALL_GETLASTSKIPPEDPAGEFAULT = 33;
	constexpr static uint32_t VMCALL_ULTIMAP_PAUSE = 34;
	constexpr static uint32_t VMCALL_ULTIMAP_RESUME = 35;
	constexpr static uint32_t VMCALL_ULTIMAP_DEBUGINFO = 36;
	constexpr static uint32_t VMCALL_PSODTEST = 37;
	//dbvm11
	constexpr static uint32_t VMCALL_GETMEM = 38;
	constexpr static uint32_t VMCALL_JTAGBREAK = 39;
	constexpr static uint32_t VMCALL_GETNMICOUNT = 40;
	constexpr static uint32_t VMCALL_WATCH_WRITES = 41;
	constexpr static uint32_t VMCALL_WATCH_READS = 42;
	constexpr static uint32_t VMCALL_WATCH_RETRIEVELOG = 43;
	constexpr static uint32_t VMCALL_WATCH_DELETE = 44;
	constexpr static uint32_t VMCALL_CLOAK_ACTIVATE = 45;
	constexpr static uint32_t VMCALL_CLOAK_DEACTIVATE = 46;
	constexpr static uint32_t VMCALL_CLOAK_READORIGINAL = 47;
	constexpr static uint32_t VMCALL_CLOAK_WRITEORIGINAL = 48;
	constexpr static uint32_t VMCALL_CLOAK_CHANGEREGONBP = 49;
	constexpr static uint32_t VMCALL_CLOAK_REMOVECHANGEREGONBP = 50;
	constexpr static uint32_t VMCALL_EPT_RESET = 51;
	constexpr static uint32_t VMCALL_LOG_CR3VALUES_START = 52;
	constexpr static uint32_t VMCALL_LOG_CR3VALUES_STOP = 53;
	constexpr static uint32_t VMCALL_REGISTERPLUGIN = 54;
	constexpr static uint32_t VMCALL_RAISEPMI = 55;
	constexpr static uint32_t VMCALL_ULTIMAP2_HIDERANGEUSAGE = 56;
	constexpr static uint32_t VMCALL_ADD_MEMORY = 57;
	constexpr static uint32_t VMCALL_DISABLE_EPT = 58;
	constexpr static uint32_t VMCALL_GET_STATISTICS = 59;
	constexpr static uint32_t VMCALL_WATCH_EXECUTES = 60;
	constexpr static uint32_t VMCALL_SETTSCADJUST = 61;
	constexpr static uint32_t VMCALL_SETSPEEDHACK = 62;
	constexpr static uint32_t VMCALL_CAUSEDDEBUGBREAK = 63;
	constexpr static uint32_t VMCALL_DISABLE_TSCADJUST = 64;
	constexpr static uint32_t VMCALL_CLOAKEX_ACTIVATE = 65;
	constexpr static uint32_t VMCALL_DISABLETSCHOOK = 66;
	constexpr static uint32_t VMCALL_ENABLETSCHOOK = 67;
	constexpr static uint32_t VMCALL_WATCH_GETSTATUS = 68;
	constexpr static uint32_t VMCALL_CLOAK_TRACEONBP = 69;
	constexpr static uint32_t VMCALL_CLOAK_TRACEONBP_REMOVE = 70;
	constexpr static uint32_t VMCALL_CLOAK_TRACEONBP_READLOG = 71;
	constexpr static uint32_t VMCALL_CLOAK_TRACEONBP_GETSTATUS = 72;
	constexpr static uint32_t VMCALL_CLOAK_TRACEONBP_STOPTRACE = 73;
	constexpr static uint32_t VMCALL_GETBROKENTHREADLISTSIZE = 74;
	constexpr static uint32_t VMCALL_GETBROKENTHREADENTRYSHORT = 75;
	constexpr static uint32_t VMCALL_GETBROKENTHREADENTRYFULL = 76;
	constexpr static uint32_t VMCALL_SETBROKENTHREADENTRYFULL = 77;
	constexpr static uint32_t VMCALL_RESUMEBROKENTHREAD = 78;
	constexpr static uint32_t VMCALL_HIDEDBVMPHYSICALADDRESSES = 79;
	constexpr static uint32_t VMCALL_HIDEDBVMPHYSICALADDRESSESALL = 80;
	constexpr static uint32_t VMCALL_KERNELMODE = 100;
	constexpr static uint32_t VMCALL_USERMODE = 101;
	constexpr static uint32_t VMCALL_DEBUG_SETSPINLOCKTIMEOUT = 254;
	//////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////
public:
	static bool IsIntel() {
		int info[4];
		__cpuid(info, 0);
		if (info[1] == 0x756e6547 && info[3] == 0x49656e69 && info[2] == 0x6c65746e)
			return true;
		return false;
	}

	static bool IsAMD() {
		int info[4];
		__cpuid(info, 0);
		if (info[1] == 0x68747541 && info[3] == 0x69746E65 && info[2] == 0x444D4163)
			return true;
		return false;
	}

	static bool IsDBVMCapable() {
		int info[4];
		if (IsIntel()) {
			__cpuid(info, 1);
			int c = info[2];
			if ((c >> 5) & 1)
				return true;
		}
		else if (IsAMD()) {
			__cpuid(info, 0x80000001);
			int c = info[2];
			if ((c >> 2) & 1)
				return true;
		}
		return false;
	}

	DBVM() : bIntel(IsIntel()) {}

	template <typename... Types>
	uint64_t VMCall(uint32_t Command, Types... Args) const {
		constexpr size_t SizeOfArgs = (0 + ... + sizeof(Args));
		uint8_t VMCallInfo[sizeof(uint32_t) + sizeof(current_password2) + sizeof(Command) + SizeOfArgs];
		size_t Index = 0;

		auto AddElement = [&]<class Type>(const Type& Element) {
			*(Type*)&VMCallInfo[Index] = Element;
			Index += sizeof(Element);
		};

		AddElement(uint32_t(sizeof(VMCallInfo)));
		AddElement(current_password2);
		AddElement(Command);
		(AddElement(Args), ...);

		if (bIntel)
			return vmcall_intel(current_password3, current_password1, VMCallInfo);
		return vmcall_amd(current_password3, current_password1, VMCallInfo);
	}

	uint32_t GetVersion() const {
		uint32_t Version;
		if (!ExceptionHandler::TryExcept([&]() { Version = (uint32_t)VMCall(VMCALL_GETVERSION); }))
			return 0;

		if ((Version >> 24) != 0xCE)
			return 0;

		return Version & 0xFFFFFF;
	}

	uintptr_t GetMemory() const {
		return VMCall(VMCALL_GETMEM);
	}

	uintptr_t SwitchToKernelMode(auto Rip, auto Param) const {
		return VMCall(VMCALL_SWITCH_TO_KERNELMODE, (uint32_t)KernelCS, (uint64_t)Rip, (uint64_t)Param);
	}

	const tReadPhysicalMemory ReadPhysicalMemory = [&](PhysicalAddress srcPA, void* dstVA, size_t size) {
		constexpr unsigned nopagefault = true;
		return VMCall(VMCALL_READ_PHYSICAL_MEMORY, srcPA, (unsigned)size, dstVA, nopagefault) == 0;
	};

	const tWritePhysicalMemory WritePhysicalMemory = [&](PhysicalAddress dstPA, const void* srcVA, size_t size) {
		constexpr unsigned nopagefault = true;
		return VMCall(VMCALL_WRITE_PHYSICAL_MEMORY, dstPA, (unsigned)size, srcVA, nopagefault) == 0;
	};

	uint64_t SwitchToKernelMode() const {
		return VMCall(VMCALL_KERNELMODE, (uint16_t)KernelCS);
	}

	void ReturnToUserMode() const {
		VMCall(VMCALL_USERMODE);
	}

	CR3 GetCR3() const {
		return VMCall(VMCALL_GETCR3);
	}

	void SetCR3(CR3 cr3) const {
		SwitchToKernelMode();
		__writecr3(cr3);
		ReturnToUserMode();
	}

	uint64_t GetCR4() const {
		return VMCall(VMCALL_GETCR4);
	}

	uint64_t ReadMSR(uint32_t MSR) const {
		SwitchToKernelMode();
		const uint64_t Result = __readmsr(MSR);
		ReturnToUserMode();
		return Result;
		//return VMCall(VMCALL_READMSR, MSR, uint64_t(0));
	}

	void WriteMSR(uint32_t MSR, uint64_t Value) const {
		SwitchToKernelMode();
		__writemsr(MSR, Value);
		ReturnToUserMode();
		//VMCall(VMCALL_WRITEMSR, MSR, Value);
	}

	bool ChangeRegisterOnBP(PhysicalAddress PABase, const ChangeRegOnBPInfo& changeregonbpinfo) const {
		CloakActivate(PABase);
		return VMCall(VMCALL_CLOAK_CHANGEREGONBP, PABase, changeregonbpinfo) == 0;
	}

	bool RemoveChangeRegisterOnBP(PhysicalAddress PABase) const {
		return VMCall(VMCALL_CLOAK_REMOVECHANGEREGONBP, PABase) == 0;
	}

	bool CloakWriteOriginal(PhysicalAddress PABase, const void* Src) const {
		return VMCall(VMCALL_CLOAK_WRITEORIGINAL, PABase, Src) == 0;
	}

	bool CloakReadOriginal(PhysicalAddress PABase, void* Dst) const {
		return VMCall(VMCALL_CLOAK_READORIGINAL, PABase, Dst) == 0;
	}

	void CloakActivate(PhysicalAddress PABase, uintptr_t Mode = 1) const {
		//1 already clocked
		//0 success
		VMCall(VMCALL_CLOAK_ACTIVATE, PABase, Mode);
	}

	void CloakDeactivate(PhysicalAddress PABase) const {
		//0 success
		VMCall(VMCALL_CLOAK_DEACTIVATE, PABase);
	}

	void CloakReset() const {
		VMCall(VMCALL_EPT_RESET);
	}

	void HideDBVM() const {
		VMCall(VMCALL_HIDEDBVMPHYSICALADDRESSESALL);
	}

	void ChangePassword(uint64_t password1, uint32_t password2, uint64_t password3) {
		VMCall(VMCALL_CHANGEPASSWORD, password1, password2, password3);
		SetPassword(password1, password2, password3);
	}

	void SetPassword(uint64_t password1, uint32_t password2, uint64_t password3) {
		current_password1 = password1;
		current_password2 = password2;
		current_password3 = password3;
	}

	void GetPassword(uint64_t& password1, uint32_t& password2, uint64_t& password3) const {
		password1 = current_password1;
		password2 = current_password2;
		password3 = current_password3;
	}

	void SetDefaultPassword() {
		current_password1 = default_password1;
		current_password2 = default_password2;
		current_password3 = default_password3;
	}

	PhysicalAddress GetPTEAddress(uintptr_t VirtualAddress, CR3 cr3) const {
		return PhysicalMemory::GetPTEAddress(VirtualAddress, cr3, ReadPhysicalMemory);
	}

	PhysicalAddress GetPhysicalAddress(uintptr_t VirtualAddress, CR3 cr3) const {
		return PhysicalMemory::GetPhysicalAddress(VirtualAddress, cr3, ReadPhysicalMemory);
	}

	bool RPM(uintptr_t Address, void* Buffer, size_t Size, CR3 cr3) const {
		return PhysicalMemory::ReadProcessMemory(Address, Buffer, Size, cr3, ReadPhysicalMemory);
	}

	bool WPM(uintptr_t Address, const void* Buffer, size_t Size, CR3 cr3) const {
		return PhysicalMemory::WriteProcessMemory(Address, Buffer, Size, cr3, ReadPhysicalMemory, WritePhysicalMemory);
	}

	bool WPMCloak(uintptr_t Address, const void* Buffer, size_t Size, CR3 cr3) const {
		return PhysicalMemory::WriteProcessMemory(Address, Buffer, Size, cr3, ReadPhysicalMemory,
			[&](PhysicalAddress PA, const void* Buffer, size_t Size) {
				PhysicalAddress PABase = PA & ~0xFFF;
				CloakActivate(PABase);

				uint8_t buf[0x1000];
				if (!CloakReadOriginal(PABase, buf))
					return false;

				memcpy(buf + PA - PABase, Buffer, Size);

				if (!CloakWriteOriginal(PABase, buf))
					return false;

				return true;
			});
	}

	bool RemoveCloak(uintptr_t Address, size_t Size, CR3 cr3) const {
		return PhysicalMemory::WriteProcessMemory(Address, 0, Size, cr3, ReadPhysicalMemory,
			[&](PhysicalAddress PA, const void* Buffer, size_t Size) {
				uintptr_t PABase = PA & ~0xFFF;
				CloakDeactivate(PABase);
				return true;
			});
	}

	bool CloakWrapper(uintptr_t Address, const void* Buffer, size_t Size, CR3 cr3, auto f) const {
		if (!WPMCloak(Address, Buffer, Size, cr3))
			return false;
		f();
		return RemoveCloak(Address, Size, cr3);
	}
};
#pragma pack(pop)
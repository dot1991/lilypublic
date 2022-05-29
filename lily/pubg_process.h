#pragma once
#include <Windows.h>
#include "common/process.h"
#include "common/exception.h"
#include "GNames.h"

extern "C" uintptr_t XenuineCall(uintptr_t Seed, uintptr_t Value, uintptr_t rax, uintptr_t pFunc);

class PubgProcess;
inline PubgProcess* g_Pubg;

class PubgProcess : public Process {
private:
	class Xenuine {
	private:
		static constexpr uintptr_t XenuineEncryptFuncAddress = 0x783DD20;
		static constexpr uintptr_t XenuineDecryptFuncAddress = XenuineEncryptFuncAddress + 0x8;

		const Process& process;
		
		uintptr_t xenuine_start_rax_encrypt = 0;
		uintptr_t xenuine_start_rax_decrypt = 0;
		uintptr_t pXenuineEncryptMapped = 0;
		uintptr_t pXenuineDecryptMapped = 0;

		bool InitEncrypt() {
			uintptr_t P;
			if (!process.ReadBase(XenuineEncryptFuncAddress, &P))
				return false;

			uint16_t Code;
			if (!process.Read(P, &Code) && Code != 0x8D48)
				return false;

			unsigned Offset;
			if (!process.Read(P + 0x3, &Offset))
				return false;

			xenuine_start_rax_encrypt = Offset + P + 0x7;
			pXenuineEncryptMapped = process.kernel.GetMappedAddress(P + 0x7);
			return true;
		}

		bool InitDecrypt() {
			uintptr_t P;
			if (!process.ReadBase(XenuineDecryptFuncAddress, &P))
				return false;

			uint16_t Code;
			if (!process.Read(P, &Code) && Code != 0x8D48)
				return false;

			unsigned Offset;
			if (!process.Read(P + 0x3, &Offset))
				return false;

			xenuine_start_rax_decrypt = Offset + P + 0x7;
			pXenuineDecryptMapped = process.kernel.GetMappedAddress(P + 0x7);
			return true;
		}

	public:
		Xenuine(const Process& process) : process(process) {
			verify(InitEncrypt());
			verify(InitDecrypt());
		}

		uintptr_t Encrypt(uintptr_t Value, uintptr_t Seed = 0) const {
			uintptr_t Result = 0;
			for (unsigned i = 0; i < 10 && !
				ExceptionHandler::TryExcept([&]() { Result = XenuineCall(Seed, Value, xenuine_start_rax_encrypt, pXenuineEncryptMapped); });
				i++, process.kernel.SetCustomCR3()) {
			}
			return Result;
		}

		uintptr_t Decrypt(uintptr_t Value, uintptr_t Seed = 0) const {
			uintptr_t Result = 0;
			for (unsigned i = 0; i < 10 && !
				ExceptionHandler::TryExcept([&]() { Result = XenuineCall(Seed, Value, xenuine_start_rax_encrypt, pXenuineDecryptMapped); });
				i++, process.kernel.SetCustomCR3()) {
			}
			return Result;
		}
	};

	HWND _hGameWnd = 0;

public:
	Xenuine xenuine = *(g_Pubg = this);
	TNameEntryArray NameArr;

	PubgProcess(Kernel& kernel, HWND hWnd) : Process(kernel, GetPIDFromHWND(hWnd)), _hGameWnd(hWnd) {}

	MAKE_GETTER(_hGameWnd, hGameWnd);
};

class XenuinePtr {
private:
	const uintptr_t P;
public:
	XenuinePtr(uintptr_t P) : P(g_Pubg->xenuine.Encrypt(P)) {}
	operator uintptr_t() const { return g_Pubg->xenuine.Decrypt(P); }
};
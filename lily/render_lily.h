#pragma once
#include "common/render_dcomp.h"
#include "common/shellcode.h"
#include "kernel_lily.h"

class RenderLily : public RenderDComp {
private:
	const KernelLily& kernel;
	LONG Width = 0, Height = 0;

	void ReleaseDirectCompositionTarget(HWND hWnd) final {
		kernel.SetOwningThreadWrapper(hWnd, [&] {
			pDirectCompositionDevice->CreateTargetForHwnd(hWnd, TOPMOST, &pDirectCompositionTarget);
			pDirectCompositionTarget.ReleaseAndGetAddressOf();
			});
	}

	bool CreateDirectCompositionTarget(HWND hWnd) final {
		const ShellCode_Ret0 ShellCode;
		bool bSuccess = false;

		kernel.SetOwningThreadWrapper(hWnd, [&] {
			pDirectCompositionDevice->CreateTargetForHwnd(hWnd, TOPMOST, &pDirectCompositionTarget);
			pDirectCompositionTarget.ReleaseAndGetAddressOf();
			if (pDirectCompositionDevice->CreateTargetForHwnd(hWnd, TOPMOST, &pDirectCompositionTarget) != S_OK)
				return;
			kernel.dbvm.CloakWrapper(kernel.LpcRequestPort, &ShellCode, sizeof(ShellCode), kernel.KrnlCR3, [&] {
				bSuccess = kernel.NtUserDestroyDCompositionHwndTarget(hWnd, TOPMOST);
				});
			});

		RECT ClientRect = {};
		GetClientRect(hWnd, &ClientRect);
		Width = ClientRect.right;
		Height = ClientRect.bottom;
		return bSuccess;
	}

	void Present(HWND hWnd) final {
		RenderDComp::Present(hWnd);

		RECT ClientRect = {};
		GetClientRect(hWnd, &ClientRect);
		if (ClientRect.right <= Width && ClientRect.bottom <= Height)
			return;

		Width = ClientRect.right;
		Height = ClientRect.bottom;
		Reset();
	}

public:
	RenderLily(const KernelLily& kernel, float DefaultFontSize) : kernel(kernel), RenderDComp(DefaultFontSize, true) {}
};
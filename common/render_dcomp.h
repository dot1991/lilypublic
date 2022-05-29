#pragma once
#include "render.h"

#include <Windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <dcomp.h>
#pragma comment(lib, "dcomp.lib")

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

class RenderDComp : public Render {
private:
	ComPtr<IDXGISwapChain1> pDXGISwapChain1;
	ComPtr<ID3D11Texture2D> pBackBuffer;
	ComPtr<IDCompositionVisual> pDirectCompositionVisual;
	HWND hAttachWnd = 0;
	float SleepTime = 0.0f;

	ImColor ClearColor() const final {
		return { 0.0f, 0.0f, 0.0f, 0.0f };
	}

	bool IsScreenPosNeeded() const final {
		return false;
	}

	virtual void ReleaseDirectCompositionTarget(HWND) {
		pDirectCompositionTarget.ReleaseAndGetAddressOf();
	}

	virtual bool CreateDirectCompositionTarget(HWND hWnd) {
		pDirectCompositionTarget.ReleaseAndGetAddressOf();
		return pDirectCompositionDevice->CreateTargetForHwnd(hWnd, TOPMOST, &pDirectCompositionTarget) == S_OK;
	}

	void DetachWindow() {
		if (!hAttachWnd)
			return;

		ReleaseDirectCompositionTarget(hAttachWnd);
		hAttachWnd = 0;
	}

	void AttachWindow(HWND hWnd) {
		if (!hWnd || !IsWindowVisible(hWnd) || IsIconic(hWnd))
			return;

		const bool bSuccess = [&] {
			HRESULT hr;

			if (!CreateDirectCompositionTarget(hWnd))
				return false;

			hr = pDirectCompositionDevice->CreateVisual(&pDirectCompositionVisual);
			if (FAILED(hr))
				return false;

			hr = pDirectCompositionVisual->SetContent(pDXGISwapChain1.Get());
			if (FAILED(hr))
				return false;

			hr = pDirectCompositionTarget->SetRoot(pDirectCompositionVisual.Get());
			if (FAILED(hr))
				return false;

			hr = pDirectCompositionDevice->Commit();
			if (FAILED(hr))
				return false;

			hr = pDirectCompositionDevice->WaitForCommitCompletion();
			if (FAILED(hr))
				return false;

			return true;
		}();

		if (!bSuccess) {
			ReleaseDirectCompositionTarget(hWnd);
			error("DComp"e);
		}

		hAttachWnd = hWnd;
	}

	bool InitDComp(bool IsProtected) {
		HRESULT hr;

		ComPtr<IDXGIFactory2> pDXGIFactory2;
		hr = CreateDXGIFactory2(0, __uuidof(pDXGIFactory2), &pDXGIFactory2);
		if (FAILED(hr))
			return false;

		const DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {
			.Width = (UINT)ScreenWidth,
			.Height = (UINT)ScreenHeight,
			.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
			.SampleDesc = {.Count = 1},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = 2,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
			.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED,
			.Flags = IsProtected ? (UINT)DXGI_SWAP_CHAIN_FLAG_HW_PROTECTED : 0
		};

		hr = pDXGIFactory2->CreateSwapChainForComposition(pDXGIDevice.Get(), &SwapChainDesc, 0, &pDXGISwapChain1);
		if (FAILED(hr))
			return false;

		hr = pDXGISwapChain1->GetBuffer(0, __uuidof(ID3D11Texture2D), &pBackBuffer);
		if (FAILED(hr))
			return false;

		hr = DCompositionCreateDevice(pDXGIDevice.Get(), __uuidof(IDCompositionDevice), &pDirectCompositionDevice);
		if (FAILED(hr))
			return false;

		return true;
	}

protected:
	constexpr static BOOL TOPMOST = TRUE;
	ComPtr<IDCompositionDevice> pDirectCompositionDevice;
	ComPtr<IDCompositionTarget> pDirectCompositionTarget;

	void Reset() {
		SleepTime = 0.5f;
		hAttachWnd = 0;
	}

	virtual void Present(HWND hWnd) {
		SleepTime = std::clamp(SleepTime - TimeDelta, 0.0f, 5.0f);
		if (SleepTime > 0.0f) {
			Sleep(1);
			return;
		}

		BOOL bEnabled = FALSE;
		DwmIsCompositionEnabled(&bEnabled);
		if (!bEnabled)
			return Reset();

		if (hWnd != hAttachWnd) {
			DetachWindow();
			AttachWindow(hWnd);
		}

		pD3D11DeviceContext->CopyResource(pBackBuffer.Get(), pD3D11Texture2D.Get());
		pDXGISwapChain1->Present(0, 0);
	}

public:
	RenderDComp(float DefaultFontSize, bool IsProtected = false) : Render(DefaultFontSize) {
		verify(InitDComp(IsProtected));
		Clear();
	}

	~RenderDComp() {
		DetachWindow();
	}
};
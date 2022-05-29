#pragma once
#include "render.h"

#include <Windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <ddraw.h> 
#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")

class RenderDDraw : public Render {
private:
	ComPtr<IDirectDrawSurface7> pPrimarySurface, pOverlaySurface, pAttachedSurface;

	ImColor ClearColor() const final {
		return { 1.0f / 255.0f, 1.0f / 255.0f, 1.0f / 255.0f, 0.0f };
	}

	bool IsScreenPosNeeded() const final {
		return true;
	}

	void Present(HWND) final {
		HDC hSurfaceDC;
		pDXGISurface->GetDC(FALSE, &hSurfaceDC);
		HDC hAttachedSurfaceDC;
		pAttachedSurface->GetDC(&hAttachedSurfaceDC);
		BitBlt(hAttachedSurfaceDC, 0, 0, ScreenWidth, ScreenHeight, hSurfaceDC, 0, 0, SRCCOPY);
		pAttachedSurface->ReleaseDC(hAttachedSurfaceDC);
		pDXGISurface->ReleaseDC(0);

		pOverlaySurface->Flip(0, DDFLIP_DONOTWAIT | DDFLIP_NOVSYNC);
	}

	bool InitDirectDraw() {
		HRESULT hr;

		ComPtr<IDirectDraw7> pDirectDraw7;
		hr = DirectDrawCreateEx(0, &pDirectDraw7, IID_IDirectDraw7, 0);
		if (FAILED(hr))
			return false;

		const bool IsDriverSupported = [&] {
			DDCAPS DDCaps = { .dwSize = sizeof(DDCAPS) };
			hr = pDirectDraw7->GetCaps(&DDCaps, 0);
			if (FAILED(hr))
				return false;

			const DWORD dwCaps = DDCaps.dwCaps;
			if (!(dwCaps & DDCAPS_COLORKEY))
				return false;

			return (dwCaps & DDCAPS_OVERLAY) || (dwCaps & DDCAPS_OVERLAYCANTCLIP);
		}();
		verify(IsDriverSupported);

		hr = pDirectDraw7->SetCooperativeLevel(0, DDSCL_NORMAL);
		if (FAILED(hr))
			return false;

		DDSURFACEDESC2 PrimarySurfaceDesc = {
			.dwSize = sizeof(PrimarySurfaceDesc),
			.dwFlags = DDSD_CAPS,
			.ddsCaps = {.dwCaps = DDSCAPS_PRIMARYSURFACE }
		};

		hr = pDirectDraw7->CreateSurface(&PrimarySurfaceDesc, &pPrimarySurface, 0);
		if (FAILED(hr))
			return false;

		DDSURFACEDESC2 OverlaySurfaceDesc = {
			.dwSize = sizeof(PrimarySurfaceDesc),
			.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_BACKBUFFERCOUNT | DDSD_PIXELFORMAT,
			.dwHeight = (unsigned long)ScreenHeight,
			.dwWidth = (unsigned long)ScreenWidth,
			.dwBackBufferCount = 1,
			.ddpfPixelFormat = DDPIXELFORMAT{ sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 32, 0xFF0000, 0x00FF00, 0x0000FF, 0 },
			.ddsCaps = {.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY }
		};

		hr = pDirectDraw7->CreateSurface(&OverlaySurfaceDesc, &pOverlaySurface, 0);
		if (FAILED(hr))
			return false;

		DDSCAPS2 AttachedSurfaceCaps2 = { .dwCaps = DDSCAPS_BACKBUFFER };
		hr = pOverlaySurface->GetAttachedSurface(&AttachedSurfaceCaps2, &pAttachedSurface);
		if (FAILED(hr))
			return false;

		DDOVERLAYFX OverlayFX = { .dwSize = sizeof(OverlayFX), .dckSrcColorkey = { ClearColor(), ClearColor() }, };
		RECT Rect = { 0, 0, ScreenWidth, ScreenHeight };
		constexpr DWORD dwUpdateFlags = DDOVER_SHOW | DDOVER_DDFX | DDOVER_KEYSRCOVERRIDE;

		for (unsigned i = 0; i < 10 &&
			FAILED(hr = pOverlaySurface->UpdateOverlay(0, pPrimarySurface.Get(), &Rect, dwUpdateFlags, &OverlayFX)); i++);

		if (FAILED(hr))
			return false;

		return true;
	}

public:
	RenderDDraw(float DefaultFontSize) : Render(DefaultFontSize) {
		verify(InitDirectDraw());
		Clear();
	}
};
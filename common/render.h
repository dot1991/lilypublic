#pragma once
#include <Windows.h>
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#include <wrl.h>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx11.h"
#include "../imgui/imgui_impl_win32.h"

#include "../ue4math/vector.h"
#include "encrypt_string.h"
#include "util.h"

class __declspec(novtable) Render {
protected:
	template<class Interface>
	using ComPtr = Microsoft::WRL::ComPtr<Interface>;

private:
	ComPtr<ID3D11Device> _pD3D11Device;
	ComPtr<ID3D11RenderTargetView> _pD3D11RenderTargetView;
	ComPtr<IDXGIDevice> _pDXGIDevice;
	ComPtr<ID3D11DeviceContext> _pD3D11DeviceContext;
	ComPtr<ID3D11Texture2D> _pD3D11Texture2D;
	ComPtr<IDXGISurface1> _pDXGISurface;

	float PosX = 0;
	float PosY = 0;
	float _Width = (float)ScreenWidth;
	float _Height = (float)ScreenHeight;

	bool _bKeyPushing[0x100] = {};
	bool _bKeyPushed[0x100] = {};
	BYTE _KeyStates[0x100] = {};

	virtual void Present(HWND hWnd);
	virtual bool IsScreenPosNeeded() const;
	virtual ImColor ClearColor() const;

	void ImGuiRenderDrawData() const {
		const ImVec4 Float4 = ClearColor();
		ImGui::Render();
		_pD3D11DeviceContext->OMSetRenderTargets(1, _pD3D11RenderTargetView.GetAddressOf(), 0);
		_pD3D11DeviceContext->ClearRenderTargetView(_pD3D11RenderTargetView.Get(), &Float4.x);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	void UpdateKeyStates() {
		for (auto i = 0; i < 0x100; i++) {
			SHORT KeyState = GetKeyState(i);
			_KeyStates[i] = (KeyState & 0x8000 ? 0x80 : 0) | (KeyState & 1 ? 1 : 0);
			_bKeyPushed[i] = IsKeyPushed(i);
			_bKeyPushing[i] = IsKeyPushing(i);
		}
	}

	void ProcessExitHotkey() {
		if (!bKeyPushing[VK_MENU] && !bKeyPushing[VK_MBUTTON])
			return;
		if (!bKeyPushed[VK_END])
			return;
		Clear();

		this->~Render();
		exit(0);
	}

	void UpdateRenderArea(HWND hWnd) {
		RECT ClientRect;
		GetClientRect(hWnd, &ClientRect);
		POINT ClientPos = { 0, 0 };
		ClientToScreen(hWnd, &ClientPos);
		PosX = (float)ClientPos.x;
		PosY = (float)ClientPos.y;
		_Width = (float)ClientRect.right;
		_Height = (float)ClientRect.bottom;
	}

	void InsertKeyInfo() const {
		if (!bRender) return;
		if (bIgnoreInput) return;

		ImGuiIO& io = ImGui::GetIO();

		//Mouse
		io.MouseDown[0] = bKeyPushing[VK_LBUTTON];
		io.MouseDown[1] = bKeyPushing[VK_RBUTTON];
		io.MouseDown[2] = bKeyPushing[VK_MBUTTON];
		io.MouseDown[3] = bKeyPushing[VK_XBUTTON1];
		io.MouseDown[4] = bKeyPushing[VK_XBUTTON2];

		POINT CursorPos;
		GetCursorPos(&CursorPos);
		io.MousePos.x = (float)CursorPos.x - PosX;
		io.MousePos.y = (float)CursorPos.y - PosY;

		//Keyboard
		memcpy(io.KeysDown, bKeyPushed, sizeof(bKeyPushed));
		io.KeyCtrl = bKeyPushing[VK_CONTROL];
		io.KeyShift = bKeyPushing[VK_SHIFT];
		io.KeyAlt = bKeyPushing[VK_MENU];
		io.KeySuper = bKeyPushing[VK_LWIN] || bKeyPushing[VK_RWIN];

		for (auto i = 0; i < _countof(bKeyPushed); i++) {
			if (!bKeyPushed[i])
				continue;

			wchar_t Keys[10] = {};
			HKL KeyboardLayout = GetKeyboardLayout(0);
			UINT ScanCode = MapVirtualKeyA(i, MAPVK_VK_TO_VSC);
			const int NumKeys = ToUnicodeEx(i, ScanCode, KeyStates, Keys, _countof(Keys), 2, KeyboardLayout);
			for (auto j = 0; j < NumKeys; j++)
				io.AddInputCharacterUTF16(Keys[j]);
		}
	}

	float _TimeSeconds = GetTimeSeconds();
	float _TimeDelta = 0.0f;

	void UpdateTime() {
		float CurrTime = GetTimeSeconds();
		_TimeDelta = CurrTime - _TimeSeconds;
		_TimeSeconds = CurrTime;
		UpdateFPS();
	}

	constexpr static float CheckFPSInterval = 1.0f;
	float TimeDeltaFPSAcc = 0.0f;
	uint32_t FPSCount = 0;
	float _FPS = 0;

	void UpdateFPS() {
		FPSCount++;
		TimeDeltaFPSAcc += TimeDelta;
		if (TimeDeltaFPSAcc < CheckFPSInterval)
			return;

		TimeDeltaFPSAcc -= CheckFPSInterval;
		_FPS = FPSCount / CheckFPSInterval;
		FPSCount = 0;
	}

	bool InitD3D() {
		HRESULT hr;

		hr = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0,
			D3D11_CREATE_DEVICE_BGRA_SUPPORT, 0, 0, D3D11_SDK_VERSION,
			&_pD3D11Device, 0, &_pD3D11DeviceContext);
		if (FAILED(hr))
			return false;

		hr = _pD3D11Device->QueryInterface(__uuidof(_pDXGIDevice), &_pDXGIDevice);
		if (FAILED(hr))
			return false;

		const D3D11_TEXTURE2D_DESC TextureDesc = {
			.Width = (UINT)ScreenWidth,
			.Height = (UINT)ScreenHeight,
			.MipLevels = 1,
			.ArraySize = 1,
			.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
			.SampleDesc = {.Count = 1 },
			.Usage = D3D11_USAGE_DEFAULT,
			.BindFlags = D3D11_BIND_RENDER_TARGET,
			.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE,
		};

		hr = _pD3D11Device->CreateTexture2D(&TextureDesc, 0, &_pD3D11Texture2D);
		if (FAILED(hr))
			return false;

		hr = _pD3D11Texture2D->QueryInterface(__uuidof(_pDXGISurface), &_pDXGISurface);
		if (FAILED(hr))
			return false;

		hr = _pD3D11Device->CreateRenderTargetView(_pD3D11Texture2D.Get(), 0, &_pD3D11RenderTargetView);
		if (FAILED(hr))
			return false;

		return true;
	}

protected:
	const int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	const int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	MAKE_GETTER(_pD3D11Device, pD3D11Device);
	MAKE_GETTER(_pD3D11RenderTargetView, pD3D11RenderTargetView);
	MAKE_GETTER(_pDXGIDevice, pDXGIDevice);
	MAKE_GETTER(_pD3D11DeviceContext, pD3D11DeviceContext);
	MAKE_GETTER(_pD3D11Texture2D, pD3D11Texture2D);
	MAKE_GETTER(_pDXGISurface, pDXGISurface);

	void Clear() {
		ImGui_ImplDX11_NewFrame();
		ImGui::NewFrame();
		ImGui::EndFrame();
		ImGuiRenderDrawData();
		Present(0);
	}

	Render(float DefaultFontSize) {
		verify(InitD3D());

		ImGui::CreateContext();
		ImGui_ImplDX11_Init(_pD3D11Device.Get(), _pD3D11DeviceContext.Get());

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = { (float)ScreenWidth , (float)ScreenHeight };

		io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf"e, DefaultFontSize);
		io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf"e, 60.0f);
		io.IniFilename = 0;

		io.KeyMap[ImGuiKey_Tab] = VK_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
		io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
		io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
		io.KeyMap[ImGuiKey_Home] = VK_HOME;
		io.KeyMap[ImGuiKey_End] = VK_END;
		io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
		io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
		io.KeyMap[ImGuiKey_Space] = VK_SPACE;
		io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
		io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
		io.KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
		io.KeyMap[ImGuiKey_A] = 'A';
		io.KeyMap[ImGuiKey_C] = 'C';
		io.KeyMap[ImGuiKey_V] = 'V';
		io.KeyMap[ImGuiKey_X] = 'X';
		io.KeyMap[ImGuiKey_Y] = 'Y';
		io.KeyMap[ImGuiKey_Z] = 'Z';
	}

	virtual ~Render() {
		ImGui_ImplDX11_Shutdown();
		ImGuiIO& io = ImGui::GetIO();
		if (io.BackendPlatformUserData)
			ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

public:
	constexpr static ImU32 COLOR_CLEAR = IM_COL32(0, 0, 0, 0);
	constexpr static ImU32 COLOR_RED = IM_COL32(255, 0, 0, 255);
	constexpr static ImU32 COLOR_GREEN = IM_COL32(0, 255, 0, 255);
	constexpr static ImU32 COLOR_BLUE = IM_COL32(0, 0, 255, 255);
	constexpr static ImU32 COLOR_BLACK = IM_COL32(0, 0, 0, 255);
	constexpr static ImU32 COLOR_WHITE = IM_COL32(255, 255, 255, 255);
	constexpr static ImU32 COLOR_ORANGE = IM_COL32(255, 128, 0, 255);
	constexpr static ImU32 COLOR_YELLOW = IM_COL32(255, 255, 0, 255);
	constexpr static ImU32 COLOR_PURPLE = IM_COL32(255, 0, 255, 255);
	constexpr static ImU32 COLOR_TEAL = IM_COL32(0, 255, 255, 255);
	constexpr static ImU32 COLOR_GRAY = IM_COL32(192, 192, 192, 255);

	bool bRender = true;
	bool bIgnoreInput = false;

	MAKE_GETTER(_Width, Width);
	MAKE_GETTER(_Height, Height);
	MAKE_GETTER(_TimeDelta, TimeDelta);
	MAKE_GETTER(_TimeSeconds, TimeSeconds);
	MAKE_GETTER(_FPS, FPS);
	MAKE_GETTER(_bKeyPushing, bKeyPushing);
	MAKE_GETTER(_bKeyPushed, bKeyPushed);
	MAKE_GETTER(_KeyStates, KeyStates);

	void RenderArea(HWND hWnd, ImColor BgColor, auto func) {
		UpdateKeyStates();
		UpdateTime();
		UpdateRenderArea(hWnd);

		ProcessExitHotkey();

		ImGuiIO& io = ImGui::GetIO();
		if (io.BackendPlatformUserData)
			ImGui_ImplWin32_NewFrame();
		else if (hWnd == GetForegroundWindow())
			InsertKeyInfo();

		ImGui_ImplDX11_NewFrame();
		ImGui::NewFrame();

		auto Viewport = ImGui::GetMainViewport();
		Viewport->Pos = IsScreenPosNeeded() ? ImVec2(-PosX, -PosY) : ImVec2(0.0f, 0.0f);
		Viewport->Size = IsScreenPosNeeded() ? ImVec2(PosX + Width, PosY + Height) : ImVec2(Width, Height);
		Viewport->WorkPos = { 0.0f, 0.0f };
		Viewport->WorkSize = { Width, Height };

		ImGui::SetNextWindowPos({ 0, 0 });
		ImGui::SetNextWindowSize({ Width, Height });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImU32)BgColor);
		ImGui::Begin("root"e, 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();

		func();

		ImGui::End();
		ImGui::EndFrame();
		ImGuiRenderDrawData();
		Present(hWnd);
	}

	static ImVec2 GetTextSize(float FontSize, const char* szText);
	void DrawString(FVector Pos, float Margin, const char* szText, float Size, ImColor Color, bool bCenterPos, bool bCenterAligned, bool bShowAlways) const;
	void DrawRatioBox(FVector from, FVector to, float HealthRatio, ImColor ColorRemain, ImColor ColorDamaged, ImColor ColorEdge) const;
	void DrawLine(FVector from, FVector to, ImColor Color, float thickness = 1.0f) const;
	void DrawCircle(FVector center, float radius, ImColor Color, int num_segments = 0, float thickness = 1.0f) const;
	void DrawCircleFilled(FVector center, float radius, ImColor Color, int num_segments = 0) const;
	void DrawRectOutlined(FVector from, FVector to, ImColor Color, float rounding = 0, ImDrawFlags flags = 0, float thickness = 1.0f) const;
	void DrawRectFilled(FVector from, FVector to, ImColor Color, float rounding = 0, ImDrawFlags flags = 0) const;
	void DrawX(FVector center, float len, ImColor Color, float thickness = 1.0f) const;
	void DrawTriangle(FVector p1, FVector p2, FVector p3, ImColor Color, float thickness = 1.0f) const;
	void DrawTriangleFilled(FVector p1, FVector p2, FVector p3, ImColor Color) const;
};
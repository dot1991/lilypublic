#include <windows.h>
#include <Psapi.h>
#include "global.h"
#include "injectorUI.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
    MODULEINFO ModuleInfo;
    GetModuleInformation((HANDLE)-1, GetModuleHandleA(0), &ModuleInfo, sizeof(ModuleInfo));
    Global::SetModuleInfo(ModuleInfo.lpBaseOfDll, ModuleInfo.SizeOfImage);

    auto szInjector = "Injector"e;

    const WNDCLASS WndClass = {
        .lpfnWndProc = WndProc,
        .hInstance = hInstance,
        .lpszClassName = szInjector
    };
    RegisterClass(&WndClass);

    HWND hWnd = CreateWindowExA(0, szInjector, ""e, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, 0, 0, hInstance, 0);

    if (!hWnd)
        return 0;

    ShowWindow(hWnd, SW_SHOW);

    RenderDComp render(30.0f);
    ImGui_ImplWin32_Init(hWnd);
    InjectorUI UI(hWnd, render);

    while (!UI.ProcessFrame()) {
        MSG msg;
        while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
            if (msg.message == WM_QUIT)
              return 0;
        }
    }

    return 0;
}
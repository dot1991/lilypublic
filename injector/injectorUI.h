#pragma once
#include "common/util.h"

#include "common/dbvm.h"
#include "common/encrypt_string.h"
#include "common/render_dcomp.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

#define INI ".\\config.ini"e
#define APP "INJECTOR"e

enum class EInjectionType {
	Normal,
	NxBitSwap,
	IntoDLL
};

class InjectorUI {
private:
	DBVM dbvm;

	bool bInjected = false;
	HWND hWnd = 0;
	RenderDComp& render;

	char szProcessName[0x100] = { 0 };
	bool bCreateProcess = false;
	char szImageName[0x100] = { 0 };
	char szIntoDLL[0x100] = { 0 };
	char szParam[0x100] = { 0 };
	EInjectionType InjectionType = EInjectionType::Normal;

	char szServiceName[0x100] = { 0 };
	char szProcessEventName[0x100] = { 0 };
	char szThreadEventName[0x100] = { 0 };
	char szSysFileName[0x100] = { 0 };

	uint64_t default_password1 = 0xf26ec13d3110be5c;
	uint32_t default_password2 = 0xbcdc7ab6;
	uint64_t default_password3 = 0xc635c7311748ef9a;

	void GetDriver64Data() {
		FILE* in = fopen("driver64.dat"e, "r"e);
		if (!in)
			return;

		int Result = fscanf(in, "%s %s %s %s %I64x %x %I64x"e,
			szServiceName, szProcessEventName, szThreadEventName, szSysFileName,
			&default_password1, &default_password2, &default_password3);

		fclose(in);
		verify(Result == 7);
	}

	bool SetPasswordFromParam();

	void OnButtonSetPassword();
	void OnButtonDBVM();
	void OnButtonInject();

	void GetDataFromINI() {
		GetPrivateProfileStringA(APP, "PROCESS"e, ""e, szProcessName, sizeof(szProcessName), INI);
		GetPrivateProfileStringA(APP, "IMAGE"e, ""e, szImageName, sizeof(szImageName), INI);
		GetPrivateProfileStringA(APP, "INTODLL"e, "", szIntoDLL, sizeof(szIntoDLL), INI);
		GetPrivateProfileStringA(APP, "PARAM"e, "", szParam, sizeof(szParam), INI);
		InjectionType = (EInjectionType)GetPrivateProfileIntA(APP, "TYPE"e, (int)EInjectionType::Normal, INI);
		bCreateProcess = GetPrivateProfileIntA(APP, "CREATE"e, false, INI);
	}

	void SaveDataToINI() const {
		WritePrivateProfileStringA(APP, "PROCESS"e, szProcessName, INI);
		WritePrivateProfileStringA(APP, "IMAGE"e, szImageName, INI);
		WritePrivateProfileStringA(APP, "INTODLL"e, szIntoDLL, INI);
		WritePrivateProfileStringA(APP, "PARAM"e, szParam, INI);
		char buffer[100];
		_itoa((int)InjectionType, buffer, 10);
		WritePrivateProfileStringA(APP, "TYPE"e, buffer, INI);
		_itoa(bCreateProcess, buffer, 10);
		WritePrivateProfileStringA(APP, "CREATE"e, buffer, INI);
	}

public:
	InjectorUI(HWND hWnd, RenderDComp& render) : hWnd(hWnd), render(render) {
		GetDataFromINI();

		szServiceName << "93827461_CEDRIVER60"e;
		szProcessEventName << "93827461_DBKProcList60"e;
		szThreadEventName << "93827461_DBKThreadList60"e;
		szSysFileName << "dbk64.sys"e;

		GetDriver64Data();

		dbvm.SetPassword(default_password1, default_password2, default_password3);
		if (!dbvm.GetVersion()) {
			dbvm.SetDefaultPassword();
			if (dbvm.GetVersion())
				dbvm.ChangePassword(default_password1, default_password2, default_password3);
		}

		SetPasswordFromParam();
	}

	~InjectorUI() {
		SaveDataToINI();
	}

	bool ProcessFrame() {
		auto CreateRightAlignedItem = [&](auto f) {
			const float ItemSpacing = ImGui::GetStyle().ItemSpacing.x;

			//The 100.0f is just a guess size for the first frame.
			static float HostButtonWidth = 100.0f;
			float pos = HostButtonWidth + ItemSpacing;
			ImGui::SameLine(ImGui::GetWindowWidth() - pos);
			f();
			//Get the actual width for next frame.
			HostButtonWidth = ImGui::GetItemRectSize().x;
		};

		constexpr auto Width = 500;

		render.RenderArea(hWnd, Render::COLOR_BLACK, [&] {
			ImGui::SetNextWindowPos({ 0, 0 });
			ImGui::SetNextWindowSize({ Width, 0 });
			ImGui::Begin("injector"e, 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

			if (dbvm.GetVersion()) {
				std::string strMsg = "dbvm loaded. "e;
				strMsg += std::to_string(dbvm.GetMemory() / 0x1000);
				strMsg += (const char*)" pages free"e;
				ImGui::Text(strMsg.c_str());
			}
			else
				ImGui::Text("dbvm not loaded"e);

			ImGui::Text("Process"e);
			ImGui::SameLine();
			float TextAlign = ImGui::GetCursorPosX();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() * 0.7f);
			ImGui::InputText("##Process"e, szProcessName, sizeof(szProcessName));
			ImGui::SameLine();
			if (ImGui::Button("GO"e, { -1, 0.0f }))
				OnButtonInject();
			
			ImGui::Text("IMAGE"e);
			ImGui::SameLine();
			ImGui::SetCursorPosX(TextAlign);
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() * 0.7f);
			ImGui::InputText("##DLL"e, szImageName, sizeof(szImageName));
			ImGui::SameLine();
			if (ImGui::Button("DBVM"e, { -1, 0.0f }))
				OnButtonDBVM();

			if (ImGui::RadioButton("Normal"e, InjectionType == EInjectionType::Normal))
				InjectionType = EInjectionType::Normal;
			ImGui::SameLine();

			CreateRightAlignedItem([&] {
				ImGui::Checkbox("CreateProcess"e, &bCreateProcess);
				});

			if (ImGui::RadioButton("NxBitSwap"e, InjectionType == EInjectionType::NxBitSwap))
				InjectionType = EInjectionType::NxBitSwap;
			if (ImGui::RadioButton("IntoDLL"e, InjectionType == EInjectionType::IntoDLL))
				InjectionType = EInjectionType::IntoDLL;

			ImGui::SameLine();
			ImGui::SetNextItemWidth(-1);
			ImGui::InputText("##IntoDLL"e, szIntoDLL, sizeof(szIntoDLL));

			ImGui::SetNextItemWidth(-1);
			ImGui::InputTextMultiline("##License"e, szParam, sizeof(szParam));

			ImGui::NewLine();

			CreateRightAlignedItem([&] {
				if (ImGui::Button("SetPassword"e))
					OnButtonSetPassword();
				});

			SetClientRect(hWnd, { 0, 0, Width, (int)ImGui::GetCursorPosY() });

			ImGui::End();
		});

		return bInjected;
	}
};
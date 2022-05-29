#include "hack.h"

void Hack::DrawHotkey() const {
	if (!bESP) return;

	if (NoticeTimeRemain == 0)
		return;

	std::string strNotice;

	auto AddOnOff = [&](const std::string& strFuncName, bool bOnOff) {
		strNotice += strFuncName;
		if (bOnOff)
			strNotice += (std::string)" : ON\n"e;
		else
			strNotice += (std::string)" : OFF\n"e;
	};

	AddOnOff("Fighter Mode(~)"e, bFighterMode);
	AddOnOff("ESP : Player(F1)"e, bPlayer);
	AddOnOff("ESP : Radar(F2)"e, bRadar);
	AddOnOff("ESP : Vehicle(F3)"e, bVehicle);
	AddOnOff("ESP : Box(F4)"e, bBox);

	strNotice += (std::string)"ESP : Item(F5) : "e;
	strNotice += nItem > 0 ? std::to_string(nItem) : "OFF"e;
	strNotice += (std::string)"\n"e;

	AddOnOff("Aimbot(F6)"e, bAimbot);
	AddOnOff("Silent Aimbot(F7)"e, bSilentAim);
	AddOnOff("Penetrate(F8)"e, bPenetrate);
	AddOnOff("MoveEnemy(F9)"e, bMoveEnemy);
	AddOnOff("AutoClick(F10)"e, bAutoClick);
	AddOnOff("TeamKill(F11)"e, bTeamKill);
	AddOnOff("Total ESP(F12)"e, bESP);

	strNotice += (std::string)"Range (+-) : "e;
	strNotice += std::to_string(nRange);
	strNotice += (std::string)"M"e;
	DrawNotice(strNotice.c_str(), Render::COLOR_TEAL);
}

void Hack::ProcessImGui() {
	if (!bESP) return;

	if (!bShowMenu)
		return;

	const float MenuSizeX = 320.0f;
	const float MenuSizeY = 550.0f;

	const float MenuPosX = render.Width - MenuSizeX - Hack::MARGIN;
	const float MenuPosY = render.Height / 2.0f - MenuSizeY / 2.0f;

	ImGui::SetNextWindowBgAlpha(0.4f);
	ImGui::SetNextWindowPos({ MenuPosX, MenuPosY }, ImGuiCond_Once);
	ImGui::SetNextWindowSize({ MenuSizeX, MenuSizeY }, ImGuiCond_Once);
	if (ImGui::Begin("Hotkey works holding mouse m-button or alt key"e))
	{
		ImGui::Text("Update : " __DATE__ " " __TIME__ ""e);
		ImGui::NewLine();

		ImGui::Text("Range (+,-)"e);
		ImGui::PushItemWidth(-1);
		ImGui::SliderInt("Range"e, &nRange, 100, 1000, "%dM"e, ImGuiSliderFlags_AlwaysClamp);
		ImGui::PopItemWidth();

		ImGui::Checkbox("Fighter Mode (~)"e, &bFighterMode);

		ImGui::RadioButton("CapsLock Mode : OFF"e, &nCapsLockMode, 0);
		ImGui::RadioButton("CapsLock Mode : TurnBack"e, &nCapsLockMode, 1);
		ImGui::RadioButton("CapsLock Mode : AimToEnemyFocusingMe"e, &nCapsLockMode, 2);

		if (ImGui::Button("LoadList"e)) {
			LoadList(BlackList, BlackListFile);
			LoadList(WhiteList, WhiteListFile);
		}

		ImGui::InputText("##UserName"e, szUserName, sizeof(szUserName));
		ImGui::SameLine();
		if (ImGui::Button("Add"e))
			AddUserToList(BlackList, BlackListFile, szUserName);
		ImGui::SameLine();
		if (ImGui::Button("Del"e))
			RemoveUserFromList(BlackList, BlackListFile, szUserName);

		ImGui::NewLine();

		if (ImGui::BeginTabBar("MyTabBar"e))
		{
			if (ImGui::BeginTabItem("ESP"e))
			{
				ImGui::Checkbox("ESP(F12)"e, &bESP);
				ImGui::Checkbox("Player(F1)"e, &bPlayer);

				if (ImGui::TreeNode("Details##PlayerESP"e)) {
					if (ImGui::BeginTable("Split3"e, 3))
					{
						ImGui::TableNextColumn(); ImGui::Checkbox("Skeleton"e, &ESP_PlayerSetting.bSkeleton);
						ImGui::TableNextColumn(); ImGui::Checkbox("Health"e, &ESP_PlayerSetting.bHealth);
						ImGui::TableNextColumn(); ImGui::Checkbox("NickName"e, &ESP_PlayerSetting.bNickName);
						ImGui::TableNextColumn(); ImGui::Checkbox("ShortNick"e, &ESP_PlayerSetting.bShortNick);
						ImGui::TableNextColumn(); ImGui::Checkbox("RankInfo"e, &ESP_PlayerSetting.bRankInfo);
						ImGui::TableNextColumn(); ImGui::Checkbox("IsKakao?"e, &ESP_PlayerSetting.bKakao);
						ImGui::TableNextColumn(); ImGui::Checkbox("Team"e, &ESP_PlayerSetting.bTeam);
						ImGui::TableNextColumn(); ImGui::Checkbox("Level"e, &ESP_PlayerSetting.bLevel);
						ImGui::TableNextColumn(); ImGui::Checkbox("Weapon"e, &ESP_PlayerSetting.bWeapon);
						ImGui::TableNextColumn(); ImGui::Checkbox("Distance"e, &ESP_PlayerSetting.bDistance);
						ImGui::TableNextColumn(); ImGui::Checkbox("Kill"e, &ESP_PlayerSetting.bKill);
						ImGui::TableNextColumn(); ImGui::Checkbox("Spectator"e, &ESP_PlayerSetting.bSpectatedCount);
						ImGui::TableNextColumn(); ImGui::Checkbox("Damage"e, &ESP_PlayerSetting.bDamage);
						ImGui::EndTable();
					}
					ImGui::TreePop();
				}

				ImGui::Checkbox("Radar(F2)"e, &bRadar);
				ImGui::Checkbox("Vehicle(F3)"e, &bVehicle);
				ImGui::Checkbox("Box(F4)"e, &bBox);
				ImGui::Checkbox("Item(F5)"e, (bool*)&nItem);
				if (nItem > 0) {
					ImGui::PushItemWidth(-1);
					ImGui::SliderInt("Item"e, &nItem, 1, 4, "%d"e, ImGuiSliderFlags_AlwaysClamp);
					ImGui::PopItemWidth();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Aimbot"e))
			{
				ImGui::Checkbox("Aimbot(F6)"e, &bAimbot);
				if (ImGui::TreeNode("Details##Aimbot"e)) {
					ImGui::Text("Fov"e);
					ImGui::PushItemWidth(-1);
					ImGui::SliderFloat("Fov"e, &AimbotFOV, AimbotFOVMin, AimbotFOVMax);
					ImGui::PopItemWidth();

					ImGui::Text("Aim Speed(Pixels Per Seconds)"e);
					ImGui::PushItemWidth(-1);
					ImGui::SliderFloat("SpeedX"e, &AimSpeedX, AimSpeedMin, AimSpeedMax, "X : %.0f"e, ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_NoRoundToFormat);
					ImGui::SliderFloat("SpeedY"e, &AimSpeedY, AimSpeedMin, AimSpeedMax, "Y : %.0f"e, ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_NoRoundToFormat);
					ImGui::PopItemWidth();
					ImGui::TreePop();
				}

				ImGui::Checkbox("Silent Aim(F7)"e, &bSilentAim);
				if (ImGui::TreeNode("Details##SilentAim"e)) {
					ImGui::Text("Fov"e);
					ImGui::PushItemWidth(-1);
					ImGui::SliderFloat("Fov"e, &SilentFOV, SilentFOVMin, SilentFOVMax);
					ImGui::PopItemWidth();
					ImGui::Text("Randomize"e);
					ImGui::PushItemWidth(-1);
					ImGui::SliderFloat("Head"e, &RandSilentAimHead, 0.0f, RandSilentAimMax, "%.1f"e, ImGuiSliderFlags_NoRoundToFormat);
					ImGui::SliderFloat("Body"e, &RandSilentAimBody, 0.0f, RandSilentAimMax, "%.1f"e, ImGuiSliderFlags_NoRoundToFormat);
					ImGui::PopItemWidth();
					ImGui::Checkbox("Dangerous Mode"e, &bSilentAim_DangerousMode);
					ImGui::TreePop();
				}

				ImGui::Checkbox("Penetrate(F8)"e, &bPenetrate);
				ImGui::Checkbox("MoveEnemy(F9)"e, &bMoveEnemy);
				ImGui::Checkbox("AutoClick(F10)"e, &bAutoClick);
				ImGui::Checkbox("TeamKill(F11)"e, &bTeamKill);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Debug"e))
			{
				ImGui::Checkbox("IgnoreInput"e, &bIgnoreInput);
				ImGui::Checkbox("Debug mode"e, &bDebug);

				ImGui::Checkbox("Capture Log"e, &bCaptureLog);
				ImGui::SameLine();
				if (ImGui::Button("Clear"e))
					debuglog.clear();
				ImGui::InputTextMultiline("##source"e, debuglog.data(), debuglog.size(), { -FLT_MIN, -FLT_MIN }, ImGuiInputTextFlags_ReadOnly);
				if (bNeedToScroll) {
					ImGui::BeginChild("##source"e);
					ImGui::SetScrollHereY(1.0f);
					bNeedToScroll = false;
					ImGui::EndChild();
				}

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Status"e))
			{
				ImGui::Text(status.c_str());
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

void Hack::ProcessHotkey() {
	if (render.bKeyPushing[VK_MENU] || render.bKeyPushing[VK_MBUTTON]) {
		if (render.bKeyPushed[VK_HOME]) {
			bShowMenu = !bShowMenu;
			bIgnoreInput = false;
		}
		if (render.bKeyPushed[VK_OEM_3]) {
			bFighterMode = !bFighterMode;
			NoticeTimeRemain = NOTICE_TIME;
		}
		if (render.bKeyPushed[VK_F1]) {
			bPlayer = !bPlayer;
			NoticeTimeRemain = NOTICE_TIME;
		}
		if (render.bKeyPushed[VK_F2]) {
			bRadar = !bRadar;
			NoticeTimeRemain = NOTICE_TIME;
		}
		if (render.bKeyPushed[VK_F3]) {
			bVehicle = !bVehicle;
			NoticeTimeRemain = NOTICE_TIME;
		}
		if (render.bKeyPushed[VK_F4]) {
			bBox = !bBox;
			NoticeTimeRemain = NOTICE_TIME;
		}
		if (render.bKeyPushed[VK_F5]) {
			nItem = (nItem + 1) % 5;
			NoticeTimeRemain = NOTICE_TIME;
		}
		if (render.bKeyPushed[VK_F6]) {
			bAimbot = !bAimbot;
			NoticeTimeRemain = NOTICE_TIME;
		}
		if (render.bKeyPushed[VK_F7]) {
			bSilentAim = !bSilentAim;
			NoticeTimeRemain = NOTICE_TIME;
		}
		if (render.bKeyPushed[VK_F8]) {
			bPenetrate = !bPenetrate;
			NoticeTimeRemain = NOTICE_TIME;
		}
		if (render.bKeyPushed[VK_F9]) {
			bMoveEnemy = !bMoveEnemy;
			NoticeTimeRemain = NOTICE_TIME;
		}
		if (render.bKeyPushed[VK_F10]) {
			bAutoClick = !bAutoClick;
			NoticeTimeRemain = NOTICE_TIME;
		}
		if (render.bKeyPushed[VK_F11]) {
			bTeamKill = !bTeamKill;
			NoticeTimeRemain = NOTICE_TIME;
		}
		if (render.bKeyPushed[VK_F12]) {
			bESP = !bESP;
			NoticeTimeRemain = NOTICE_TIME;
		}
		if (render.bKeyPushed[VK_ADD]) {
			nRange += 100;
			nRange -= nRange % 100;
			if (nRange > 1000)
				nRange = 1000;

			NoticeTimeRemain = NOTICE_TIME;
		}
		if (render.bKeyPushed[VK_SUBTRACT]) {
			if (nRange % 100 == 0)
				nRange -= 100;
			nRange -= nRange % 100;
			if (nRange < 100)
				nRange = 100;

			NoticeTimeRemain = NOTICE_TIME;
		}
	}
}
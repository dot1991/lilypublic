#include "hack.h"
#include <map>

#include "GNames.h"
#include "GObjects.h"

#include "info_bone.h"
#include "info_vehicle.h"
#include "info_object.h"
#include "info_package.h"
#include "info_character.h"

enum class CharacterState {
	Dead,
	Alive,
	Groggy
};

void Hack::Loop() {
	LoadList(BlackList, BlackListFile);
	LoadList(WhiteList, WhiteListFile);

	const HWND hGameWnd = pubg.hGameWnd;
	const CR3 mapCR3 = kernel.GetMapCR3();
	//FUObjectArray ObjectArr;
	//ObjectArr.DumpObject();

	const FName KeyMouseX("MouseX"e);
	verify(KeyMouseX.ComparisonIndex);
	const FName KeyMouseY("MouseY"e);
	verify(KeyMouseY.ComparisonIndex);

	//41 0f ? ? 73 ? f3 0f 10 ? ? ? ? ? f3 0f 11
	//first result
	constexpr uintptr_t HookBaseAddress = 0x4AF232;
	const uintptr_t AimHookAddressVA = pubg.GetBaseAddress() + HookBaseAddress;
	const PhysicalAddress AimHookAddressPA = dbvm.GetPhysicalAddress(AimHookAddressVA, mapCR3);
	verify(AimHookAddressPA);
	dbvm.CloakActivate(AimHookAddressPA);

	//e8 ? ? ? ? f2 0f 10 00 f2 0f ? ? ? ? ? 00 00 8b 40 08 89 ? ? ? ? 00 00 48
	constexpr uintptr_t GunLocScopeHookBaseAddress = 0x4AE113;
	const uintptr_t GunLocScopeHookAddressVA = pubg.GetBaseAddress() + GunLocScopeHookBaseAddress;
	const PhysicalAddress GunLocScopeHookAddressPA1 = dbvm.GetPhysicalAddress(GunLocScopeHookAddressVA, mapCR3);
	const PhysicalAddress GunLocScopeHookAddressPA2 = dbvm.GetPhysicalAddress(GunLocScopeHookAddressVA + 0xC, mapCR3);
	verify(GunLocScopeHookAddressPA1);
	dbvm.CloakActivate(GunLocScopeHookAddressPA1);
	verify(GunLocScopeHookAddressPA2);
	dbvm.CloakActivate(GunLocScopeHookAddressPA2);

	//74 ? 48 8d ? ? ? ? 00 00 e8 ? ? ? ? eb ? 48 8d ? ? ? ? 00 00 e8 ? ? ? ? f2 0f 10 00 f2 0f
	constexpr uintptr_t GunLocNoScopeHookBaseAddress = 0x4ADA5D;
	const uintptr_t GunLocNoScopeHookAddressVA = pubg.GetBaseAddress() + GunLocNoScopeHookBaseAddress;
	const PhysicalAddress GunLocNoScopeHookAddressPA1 = dbvm.GetPhysicalAddress(GunLocNoScopeHookAddressVA, mapCR3);
	const PhysicalAddress GunLocNoScopeHookAddressPA2 = dbvm.GetPhysicalAddress(GunLocNoScopeHookAddressVA + 0xC, mapCR3);
	verify(GunLocNoScopeHookAddressPA1);
	dbvm.CloakActivate(GunLocNoScopeHookAddressPA1);
	verify(GunLocNoScopeHookAddressPA2);
	dbvm.CloakActivate(GunLocNoScopeHookAddressPA2);

	//f6 84 ? ? ? ? ? 01 74 ? f3 0f 10
	//first result
	constexpr uintptr_t GunLocNearWallHookBaseAddress = 0x4B01E8;
	const uintptr_t GunLocNearWallHookAddressVA = pubg.GetBaseAddress() + GunLocNearWallHookBaseAddress;
	const PhysicalAddress GunLocNearWallHookAddressPA = dbvm.GetPhysicalAddress(GunLocNearWallHookAddressVA, mapCR3);
	verify(GunLocNearWallHookAddressPA);
	dbvm.CloakActivate(GunLocNearWallHookAddressPA);

	float TimeDeltaAcc = 0.0f;
	float LastAimUpdateTime = 0.0f;
	float RemainMouseX = 0.0f;
	float RemainMouseY = 0.0f;
	NativePtr<ATslCharacter> CachedMyTslCharacterPtr = 0;
	NativePtr<ATslCharacter> LockAimbotTargetPtr = 0;
	NativePtr<ATslCharacter> LockClosestTargetPtr = 0;
	NativePtr<ATslCharacter> EnemyFocusingMePtr = 0;
	bool bPushedCapsLock = false;
	bool IsFPPOnly = true;
	bool bPrevLobby = true;

	bool bRadarExtended = false;
	float LastRadarDistanceUpdateTime = 0.0f;
	float LastRadarDistance = 200.0f;
	float SavedRadarDistance = 200.0f;

	struct CharacterInfo {
		NativePtr<ATslCharacter> Ptr;
		float Distance = 0.0f;
		int Team = -1;
		int SpectatedCount = 0;
		bool IsDisconnected = false;
		bool IsBlackListed = false;
		bool IsWhiteListed = false;
		bool IsFPP = false;
		bool IsWeaponed = false;
		float TimeAfterShot = 10.0f;
		bool IsReloading = false;
		bool IsWeaponReady = false;
		bool IsAutoFiring = false;
		bool IsProperForAutoClick = false;
		bool IsFocusingMe = false;
		FRotator AimOffsets;
		FRotator LastFiringRot;
		bool IsScoping = false;
		bool IsVisible = false;
		bool IsAI = false;
		float Health = 0.0f;
		float GroggyHealth = 0.0f;
		CharacterState State = CharacterState::Dead;
		float ZeroingDistance = 100.0f;
		NativePtr<UCurveVector> BallisticCurve = 0;
		float Gravity = -9.8f;
		float BDS = 1.0f;
		float VDragCoefficient = 1.0f;
		float SimulationSubstepTime = 0.016f;
		float InitialSpeed = 800.0f;
		float BulletDropAdd = 7.0f;
		FVector RootLocation;
		FVector Location;
		FVector AimPoint;
		bool IsLocked = false;
		FRotator Recoil;
		FRotator ControlRotation;
		FVector GunLocation;
		FRotator GunRotation;
		FVector AimLocation;
		FRotator AimRotation;
		FVector Velocity;

		bool IsInVehicle = false;
		int NumKills = 0;
		float Damage = 0.0f;
		int Level = -1;
		std::map<int, FVector> BonesPos, BonesScreenPos;
		std::string PlayerName;
		std::string WeaponName;
		tWeaponType WeaponType = tWeaponType::None;
		int Ammo = -1;
		std::string Weapon1Name;
		std::string Weapon2Name;

		std::string GetWeaponInfoStr() const {
			if (WeaponName.empty())
				return {};

			if (Ammo == -1)
				return WeaponName;

			return WeaponName + sformat((const char*)"({})"e, IsReloading ? (std::string)"..."e : std::to_string(Ammo));;
		}

		float GetSpeedXY() const { return FVector(Velocity.X, Velocity.Y, 0.0f).Length(); }
		float GetSpeedXYPerHour() const { return GetSpeedXY() / 100.0f * 3.6f; }
	};

	struct tMapInfo {
		float TimeStamp = 0.0f;

		struct {
			struct PosInfo {
				float Time = 0;
				FVector Pos;
			};
			std::deque<PosInfo> Info;
		}PosInfo;

		struct {
			int Ammo = -1;
			float TimeAfterShot = 10.0f;
			FRotator AimOffsets;
		}FiringInfo;

		struct {
			bool IsLocked = false;
			FVector AimPoint;
			FVector Velocity;
			CharacterState State = CharacterState::Dead;
			bool IsInVehicle = false;
		}AimbotInfo;

		float DisconnectedTime = 0.0f;
		float FocusTime = 0.0f;
	};

	std::map<uintptr_t, tMapInfo> EnemyInfoMap;

	while (IsWindow(hGameWnd)) {
		const HWND hForeWnd = GetForegroundWindow();
		const bool bGameForeground = (hGameWnd == hForeWnd);
		if (bGameForeground)
			ProcessHotkey();

		NoticeTimeRemain = std::clamp(NoticeTimeRemain - render.TimeDelta, 0.0f, NOTICE_TIME);

		bool bDeadByEject = false;
		auto FuncInRenderArea = [&]() {
			ProcessImGui();
			DrawHotkey();
			DrawFPS(render.FPS, Render::COLOR_TEAL);
			UserInfo.Update();
			status.clear();
			////////////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////////
			constexpr float BallisticDragScale = 1.0f;
			constexpr float BallisticDropScale = 1.0f;
			float WorldTimeSeconds = 0.0f;
			bool IsNeedToHookAim = false;
			bool IsNeedToHookGunLoc = false;
			TArray<NativePtr<AActor>> Actors;
			FVector CameraLocation;
			FRotator CameraRotation;
			FMatrix CameraRotationMatrix;
			float CameraFOV = 0.0f;
			float DefaultFOV = 0.0f;
			float MouseXSensitivity = 0.02f;
			float MouseYSensitivity = 0.02f;
			const bool IsAutoClickOn = bAutoClick && render.bKeyPushing[VK_LBUTTON];

			CharacterInfo MyInfo;
			auto IsPenetrateOn = [&] { return bPenetrate && render.bKeyPushing[VK_CONTROL] && !MyInfo.IsInVehicle; };

			auto WorldToScreen = [&](const FVector& WorldLocation) {
				return this->WorldToScreen(WorldLocation, CameraRotationMatrix, CameraLocation, CameraFOV);
			};
			auto DrawRatioBox = [&](const FVector& ScreenPos, float CameraDistance, float BarLength3D, float Ratio, ImColor ColorRemain, ImColor ColorDamaged, ImColor ColorEdge) {
				FVector ZeroLocation;
				FMatrix ZeroRotationMatrix = FRotator().GetMatrix();
				FVector ScreenPos1 = this->WorldToScreen({ CameraDistance, BarLength3D / 2.0f, 0.0f }, ZeroRotationMatrix, ZeroLocation, CameraFOV);
				FVector ScreenPos2 = this->WorldToScreen({ CameraDistance, -BarLength3D / 2.0f, 0.0f }, ZeroRotationMatrix, ZeroLocation, CameraFOV);

				float ScreenLengthX = std::clamp(ScreenPos1.X - ScreenPos2.X, render.Width / 64.0f, render.Width / 16.0f);
				float ScreenLengthY = std::clamp(ScreenLengthX / 6.0f, 5.0f, 6.0f);

				render.DrawRatioBox(
					{ ScreenPos.X - ScreenLengthX / 2.0f, ScreenPos.Y - ScreenLengthY / 2.0f, ScreenPos.Z },
					{ ScreenPos.X + ScreenLengthX / 2.0f, ScreenPos.Y + ScreenLengthY / 2.0f, ScreenPos.Z },
					Ratio, ColorRemain, ColorDamaged, ColorEdge);

				return ImVec2{ ScreenLengthX, ScreenLengthY };
			};
			auto DrawItem = [&](NativePtr<UItem> ItemPtr, FVector ItemLocation) {
				UItem Item;
				if (!ItemPtr.Read(Item))
					return false;

				auto ItemInfo = Item.GetInfo();
				const std::string ItemName = bDebug || ItemInfo.Name.empty() ? Item.GetItemID().GetName() : ItemInfo.Name.data();
				if (ItemName.empty())
					return false;

				const int ItemPriority = ItemInfo.ItemPriority;
				if (ItemPriority < nItem && !bDebug)
					return false;

				const ImColor ItemColor = [&] {
					if (ItemPriority < nItem)
						return Render::COLOR_WHITE;
					switch (ItemPriority) {
					case 1: return Render::COLOR_YELLOW;
					case 2: return Render::COLOR_ORANGE;
					case 3: return Render::COLOR_PURPLE;
					case 4: return Render::COLOR_TEAL;
					case 5: return Render::COLOR_BLACK;
					default:return Render::COLOR_WHITE;
					}
				}();

				DrawString(ItemLocation, ItemName, ItemColor, false);
				return true;
			};
			auto GetCharacterInfo = [&](NativePtr<ATslCharacter> CharacterPtr, CharacterInfo& Info) -> bool {
				const bool IsMe = &Info == &MyInfo;
				const unsigned NameHash = CharacterPtr.GetHash();
				if (!IsPlayerCharacter(NameHash) && !IsAICharacter(NameHash))
					return false;

				ATslCharacter TslCharacter;
				if (!CharacterPtr.Read(TslCharacter))
					return false;

				USceneComponent RootComponent;
				if (!TslCharacter.RootComponent.Read(RootComponent))
					return false;

				Info.RootLocation = RootComponent.ComponentToWorld.Translation;
				Info.Velocity = RootComponent.ComponentVelocity;

				USkeletalMeshComponent Mesh;
				if (!TslCharacter.Mesh.Read(Mesh))
					return false;

				UTslAnimInstance TslAnimInstance;
				if (!Mesh.AnimScriptInstance.ReadOtherType(TslAnimInstance))
					return false;

				//MoveEnemy
				[&] {
					if (IsMe)
						return;

					if (!bMoveEnemy)
						return;

					if (!render.KeyStates[VK_CAPITAL]) {
						nEnemyMoveDir = Direction::None;
						return;
					}

					if (render.bKeyPushing[VK_UP] && render.bKeyPushing[VK_LEFT])		nEnemyMoveDir = Direction::UpLeft;
					if (render.bKeyPushing[VK_UP] && render.bKeyPushing[VK_RIGHT])		nEnemyMoveDir = Direction::UpRight;
					if (render.bKeyPushing[VK_DOWN] && render.bKeyPushing[VK_LEFT])		nEnemyMoveDir = Direction::DownLeft;
					if (render.bKeyPushing[VK_DOWN] && render.bKeyPushing[VK_RIGHT])	nEnemyMoveDir = Direction::DownRight;
					if (render.bKeyPushed[VK_UP])		nEnemyMoveDir = Direction::Up;
					if (render.bKeyPushed[VK_LEFT])		nEnemyMoveDir = Direction::Left;
					if (render.bKeyPushed[VK_DOWN])		nEnemyMoveDir = Direction::Down;
					if (render.bKeyPushed[VK_RIGHT])	nEnemyMoveDir = Direction::Right;

					FVector Dir = [&]()->FVector {
						FVector Base = Info.RootLocation - MyInfo.RootLocation;
						FVector Up = { 0.0f, 0.0f, 1.0f };
						FVector Down = { 0.0f, 0.0f, -1.0f };
						FVector Left = FVector(Base.Y, -Base.X, 0.0f).GetNormalizedVector();
						FVector Right = -Left;
						switch (nEnemyMoveDir) {
						case Direction::Up:			return Up;
						case Direction::Down:		return Down;
						case Direction::Left:		return Left;
						case Direction::Right:		return Right;
						case Direction::UpLeft:		return (Up + Left) * 0.5f;
						case Direction::UpRight:	return (Up + Right) * 0.5f;
						case Direction::DownLeft:	return (Down + Left) * 0.5f;;
						case Direction::DownRight:	return (Down + Right) * 0.5f;
						default:					return -Base;
						}
					}();
					Dir.Normalize();

					Mesh.ComponentToWorld.Translation =
						Info.RootLocation + TslCharacter.BaseTranslationOffset + Dir * MoveEnemyDistance;

					pubg.Write(TslCharacter.Mesh +
						offsetof(USceneComponent, ComponentToWorld) +
						offsetof(FTransform, Translation),
						&Mesh.ComponentToWorld.Translation);
				}();

				Info.Location = Mesh.ComponentToWorld.Translation;
				Info.Distance = IsMe ? 0.0f : MyInfo.Location.Distance(Info.Location) / 100.0f;
				Info.IsVisible = Mesh.IsVisible() || IsPenetrateOn();

				//Bones
				auto BoneSpaceTransforms = Mesh.BoneSpaceTransforms.GetVector();
				size_t BoneSpaceTransformsSize = BoneSpaceTransforms.size();
				if (!BoneSpaceTransformsSize)
					return false;

				for (auto BoneIndex : GetBoneIndexArray()) {
					verify(BoneIndex < BoneSpaceTransformsSize);
					FVector Pos = (BoneSpaceTransforms[BoneIndex] * Mesh.ComponentToWorld).Translation;
					Info.BonesPos[BoneIndex] = Pos;
					Info.BonesScreenPos[BoneIndex] = WorldToScreen(Pos);
				}

				Info.Ptr = CharacterPtr;
				Info.IsAI = IsAICharacter(NameHash);
				Info.AimOffsets = TslCharacter.AimOffsets;
				Info.Health = TslCharacter.Health / TslCharacter.HealthMax;
				Info.GroggyHealth = TslCharacter.GroggyHealth / TslCharacter.GroggyHealthMax;
				Info.Team = TslCharacter.LastTeamNum;
				Info.SpectatedCount = TslCharacter.SpectatedCount;
				Info.IsScoping = TslAnimInstance.bIsScoping_CP;
				Info.Recoil = TslAnimInstance.RecoilADSRotation_CP;
				Info.Recoil.Yaw += (TslAnimInstance.LeanRightAlpha_CP - TslAnimInstance.LeanLeftAlpha_CP) * Info.Recoil.Pitch / 3.0f;
				Info.ControlRotation = TslAnimInstance.ControlRotation_CP + Info.Recoil;
				Info.IsFPP = TslAnimInstance.bLocalFPP_CP;
				Info.State =
					Info.Health > 0.0f ? CharacterState::Alive :
					Info.GroggyHealth > 0.0f ? CharacterState::Groggy :
					CharacterState::Dead;

				Info.PlayerName = ws2s(TslCharacter.CharacterName.GetString());
				Info.IsBlackListed = IsUserInList(BlackList, Info.PlayerName.c_str());
				Info.IsWhiteListed = IsUserInList(WhiteList, Info.PlayerName.c_str());

				//PlayerState
				[&] {
					if (!TslCharacter.PlayerState)
						return;

					ATslPlayerState TslPlayerState;
					if (!TslCharacter.PlayerState.ReadOtherType(TslPlayerState))
						return;

					Info.NumKills = TslPlayerState.PlayerStatistics.NumKills;
					Info.Damage = TslPlayerState.DamageDealtOnEnemy;
					Info.Level = TslPlayerState.PubgIdData.SurvivalLevel;
				}();

				//Vehicle
				[&] {
					UVehicleRiderComponent VehicleRiderComponent;
					if (!TslCharacter.VehicleRiderComponent.Read(VehicleRiderComponent))
						return;

					if (VehicleRiderComponent.SeatIndex == -1)
						return;

					APawn LastVehiclePawn;
					if (!VehicleRiderComponent.LastVehiclePawn.Read(LastVehiclePawn))
						return;

					Info.IsInVehicle = true;
					Info.Velocity = LastVehiclePawn.ReplicatedMovement.LinearVelocity;

					ATslPlayerState TslPlayerState;
					if (!LastVehiclePawn.PlayerState.ReadOtherType(TslPlayerState))
						return;

					Info.NumKills = TslPlayerState.PlayerStatistics.NumKills;
					Info.Damage = TslPlayerState.DamageDealtOnEnemy;
					Info.Level = TslPlayerState.PubgIdData.SurvivalLevel;
				}();

				Info.AimPoint =
					render.bKeyPushing[VK_SHIFT] ?
					Info.BonesPos[forehead] :
					Info.IsInVehicle ?
					Info.BonesPos[neck_01] * 0.75f + Info.BonesPos[spine_02] * 0.25f :
					Info.BonesPos[neck_01] * 0.5f + Info.BonesPos[spine_02] * 0.5f;

				//WeaponInfo
				[&] {
					ATslWeapon TslWeapon;
					if (!TslCharacter.GetTslWeapon(TslWeapon))
						return;

					auto WeaopnInfo = TslWeapon.GetWeaponInfo();
					Info.WeaponName = WeaopnInfo.WeaponName.data();
					Info.WeaponType = WeaopnInfo.WeaponType;
					if (Info.WeaponName.empty() && bDebug)
						Info.WeaponName = TslWeapon.GetFName().GetName();
				}();

				//Weapon
				[&] {
					ATslWeapon_Trajectory TslWeapon_Trajectory;
					if (TslCharacter.GetTslWeapon_Trajectory_Next(TslWeapon_Trajectory, 0)) {
						auto WeaopnInfo = TslWeapon_Trajectory.GetWeaponInfo();
						Info.Weapon1Name = WeaopnInfo.WeaponName.data();
					}

					if (TslCharacter.GetTslWeapon_Trajectory_Next(TslWeapon_Trajectory, 1)) {
						auto WeaopnInfo = TslWeapon_Trajectory.GetWeaponInfo();
						Info.Weapon2Name = WeaopnInfo.WeaponName.data();
					}

					ATslWeapon_Trajectory TslWeapon;
					if (!TslCharacter.GetTslWeapon_Trajectory(TslWeapon))
						return;

					Info.IsAutoFiring = TslWeapon.CurrentState == EWeaponState::EWeaponState__Firing;
					Info.IsReloading = TslWeapon.CurrentState == EWeaponState::EWeaponState__Reloading;
					Info.IsWeaponReady = TslWeapon.bWeaponCycleDone;
					Info.Ammo = TslWeapon.GetCurrentAmmo();
					Info.IsWeaponed = true;
					Info.Gravity = TslWeapon.TrajectoryGravityZ;
					Info.ZeroingDistance = TslWeapon.GetZeroingDistance(Info.IsScoping);
					Info.IsProperForAutoClick = TslWeapon.IsProperForAutoClick(Info.IsScoping);

					UWeaponTrajectoryData WeaponTrajectoryData;
					if (TslWeapon.WeaponTrajectoryData.Read(WeaponTrajectoryData)) {
						Info.BDS = WeaponTrajectoryData.TrajectoryConfig.BDS;
						Info.VDragCoefficient = WeaponTrajectoryData.TrajectoryConfig.VDragCoefficient;
						Info.SimulationSubstepTime = WeaponTrajectoryData.TrajectoryConfig.SimulationSubstepTime;
						Info.BallisticCurve = WeaponTrajectoryData.TrajectoryConfig.BallisticCurve;
						Info.InitialSpeed = WeaponTrajectoryData.TrajectoryConfig.InitialSpeed;
					}

					UWeaponMeshComponent WeaponMesh;
					if (TslWeapon.Mesh3P.Read(WeaponMesh)) {
						FTransform GunTransform = WeaponMesh.GetSocketTransform(TslWeapon.FiringAttachPoint, RTS_World);
						Info.GunLocation = GunTransform.Translation;
						Info.GunRotation = GunTransform.Rotation;
						Info.BulletDropAdd = WeaponMesh.GetScopingAttachPointRelativeZ(TslWeapon.ScopingAttachPoint) -
							GunTransform.GetRelativeTransform(WeaponMesh.ComponentToWorld).Translation.Z;
					}
				}();

				Info.AimLocation = Info.GunLocation.Length() > 0.0f ? Info.GunLocation : Info.Location;
				Info.AimRotation = Info.IsScoping ? Info.GunRotation : Info.ControlRotation;

				float TimeStampDelta = WorldTimeSeconds - EnemyInfoMap[CharacterPtr].TimeStamp;
				EnemyInfoMap[CharacterPtr].TimeStamp = WorldTimeSeconds;

				//PosInfo
				[&] {
					auto& PosInfo = EnemyInfoMap[CharacterPtr].PosInfo.Info;

					if (Info.State == CharacterState::Dead || !Info.IsVisible) {
						PosInfo.clear();
						return;
					}

					if (TimeStampDelta)
						PosInfo.push_front({ WorldTimeSeconds, Info.Location });

					if (PosInfo.size() > 200)
						PosInfo.pop_back();

					float SumTimeDelta = 0.0f;
					FVector SumPosDif;

					for (size_t i = 1; i < PosInfo.size(); i++) {
						const float DeltaTime = PosInfo[i - 1].Time - PosInfo[i].Time;
						const FVector DeltaPos = PosInfo[i - 1].Pos - PosInfo[i].Pos;
						const FVector DeltaVelocity = DeltaPos * (1.0f / DeltaTime);
						const float DeltaSpeedPerHour = DeltaVelocity.Length() / 100.0f * 3.6f;

						if (DeltaTime > 0.05f || DeltaSpeedPerHour > 500.0f) {
							PosInfo.clear();
							return;
						}

						SumTimeDelta = SumTimeDelta + DeltaTime;
						SumPosDif = SumPosDif + DeltaPos;

						if (SumTimeDelta > 0.15f)
							break;
					}

					if (SumTimeDelta < 0.1f)
						return;

					Info.Velocity = SumPosDif * (1.0f / SumTimeDelta);
				}();

				//DisconnectedInfo
				bool IsDisconnected = [&] {
					if (Info.IsAI)
						return false;
					if (Info.PlayerName.empty())
						return true;
					if (Info.AimOffsets.Length() == 0.0f)
						return true;
					return false;
				}();

				float& DisconnectedTime = EnemyInfoMap[CharacterPtr].DisconnectedTime;
				DisconnectedTime = IsDisconnected ? DisconnectedTime + TimeStampDelta : 0.0f;
				if (DisconnectedTime > 2.0f)
					Info.IsDisconnected = true;

				//FiringInfo
				auto& FiringInfo = EnemyInfoMap[CharacterPtr].FiringInfo;

				int PrevAmmo = FiringInfo.Ammo;
				if (Info.Ammo != -1 && PrevAmmo != -1 && Info.Ammo == PrevAmmo - 1) {
					FiringInfo.TimeAfterShot = 0.0f;
					FiringInfo.AimOffsets = Info.AimOffsets;
				}
				else
					FiringInfo.TimeAfterShot += TimeStampDelta;

				Info.TimeAfterShot = FiringInfo.TimeAfterShot;
				Info.LastFiringRot = FiringInfo.AimOffsets;
				FiringInfo.Ammo = Info.Ammo;

				//AimbotInfo
				auto& AimbotInfo = EnemyInfoMap[CharacterPtr].AimbotInfo;

				if (CharacterPtr == LockAimbotTargetPtr) {
					if (AimbotInfo.IsLocked) {
						if (AimbotInfo.IsInVehicle)
							AimbotInfo.AimPoint = AimbotInfo.AimPoint + AimbotInfo.Velocity * TimeStampDelta;
						Info.AimPoint = AimbotInfo.AimPoint;
						Info.Velocity = AimbotInfo.Velocity;
						Info.IsVisible = true;
					}
					else {
						if (AimbotInfo.State == CharacterState::Alive && Info.State == CharacterState::Groggy)
							AimbotInfo.IsLocked = true;
						else if (AimbotInfo.State == CharacterState::Alive && Info.State == CharacterState::Dead)
							AimbotInfo.IsLocked = true;
						else if (AimbotInfo.State == CharacterState::Groggy && Info.State == CharacterState::Dead)
							AimbotInfo.IsLocked = true;
						else {
							AimbotInfo.AimPoint = Info.AimPoint;
							AimbotInfo.Velocity = Info.Velocity;
							AimbotInfo.IsInVehicle = Info.IsInVehicle;
						}
					}
				}
				else
					AimbotInfo.IsLocked = false;

				Info.IsLocked = AimbotInfo.IsLocked;
				AimbotInfo.State = Info.State;

				//FocusingInfo
				[&] {
					if (IsMe)
						return;

					float AccTime = EnemyInfoMap[Info.Ptr].FocusTime;
					EnemyInfoMap[Info.Ptr].FocusTime = 0.0f;

					if (MyInfo.Health <= 0.0f)
						return;
					if (Info.Health <= 0.0f)
						return;
					if (!Info.IsWeaponed)
						return;
					if (Info.AimOffsets.Length() == 0.0f)
						return;
					if (!bTeamKill && (Info.Team == MyInfo.Team || Info.IsWhiteListed))
						return;

					auto Result = GetBulletDropAndTravelTime(
						Info.GunLocation,
						Info.AimOffsets,
						MyInfo.Location,
						Info.ZeroingDistance,
						Info.BulletDropAdd,
						Info.InitialSpeed,
						Info.Gravity,
						BallisticDragScale,
						BallisticDropScale,
						Info.BDS,
						Info.SimulationSubstepTime,
						Info.VDragCoefficient,
						Info.BallisticCurve);

					float BulletDrop = Result.first;
					float TravelTime = Result.second;

					float MinPitch = FLT_MAX;
					float MaxPitch = -FLT_MAX;
					float MinYaw = FLT_MAX;
					float MaxYaw = -FLT_MAX;

					for (auto BoneIndex : GetBoneIndexArray()) {
						FVector PredictedPos = MyInfo.BonesPos[BoneIndex];
						PredictedPos.Z += BulletDrop;
						PredictedPos = PredictedPos + MyInfo.Velocity * TravelTime;
						FRotator Rotator = (PredictedPos - Info.GunLocation).GetDirectionRotator();
						Rotator.Clamp();
						MinPitch = std::clamp(Rotator.Pitch, -FLT_MAX, MinPitch);
						MaxPitch = std::clamp(Rotator.Pitch, MaxPitch, FLT_MAX);
						MinYaw = std::clamp(Rotator.Yaw, -FLT_MAX, MinYaw);
						MaxYaw = std::clamp(Rotator.Yaw, MaxYaw, FLT_MAX);
					}

					float CenterYaw = (MinYaw + MaxYaw) / 2.0f;
					float RangeYaw = (MaxYaw - MinYaw) / 2.0f;
					float RangeYawAdd = RangeYaw + std::clamp(RangeYaw * 1.5f, 1.0f, FLT_MAX);
					float RangeYawMinus = RangeYawAdd;

					float CenterPitch = (MinPitch + MaxPitch) / 2.0f;
					float RangePitch = (MaxPitch - MinPitch) / 2.0f;
					float RangePitchAdd = RangePitch + std::clamp(RangePitch * 2.0f, 5.0f, FLT_MAX);
					float RangePitchMinus = RangePitch + std::clamp(RangePitch * 1.5f, 1.0f, FLT_MAX);

					FRotator AimOffsets = Info.AimOffsets;
					AimOffsets.Clamp();

					if (AimOffsets.Yaw < CenterYaw - RangeYawMinus)
						return;
					if (AimOffsets.Yaw > CenterYaw + RangeYawAdd)
						return;
					if (AimOffsets.Pitch < CenterPitch - RangePitchMinus)
						return;
					if (AimOffsets.Pitch > CenterPitch + RangePitchAdd)
						return;

					EnemyInfoMap[Info.Ptr].FocusTime = AccTime + TimeStampDelta;
					if (EnemyInfoMap[Info.Ptr].FocusTime > MinFocusTime)
						Info.IsFocusingMe = true;
				}();

				return true;
			};

			NativePtr<UObject> MyPawnPtr = 0;

			[&] {
				UWorld World;
				if (!UWorld::GetUWorld(World))
					return;

				WorldTimeSeconds = World.TimeSeconds;

				status += (std::string)"WorldTime : "e + std::to_string((unsigned)WorldTimeSeconds) + (std::string)"\n"e;

				ULevel Level;
				if (!World.CurrentLevel.Read(Level))
					return;

				if (!Level.Actors.Read(Actors))
					return;

				UGameInstance GameInstance;
				if (!World.GameInstance.Read(GameInstance))
					return;

				EncryptedPtr<ULocalPlayer> LocalPlayerPtr;
				if (!GameInstance.LocalPlayers.GetValue(0, LocalPlayerPtr))
					return;

				ULocalPlayer LocalPlayer;
				if (!LocalPlayerPtr.Read(LocalPlayer))
					return;

				ATslPlayerController PlayerController;
				if (!LocalPlayer.PlayerController.ReadOtherType(PlayerController))
					return;
				
				ATslBaseHUD TslBaseHUD;
				if (!PlayerController.MyHUD.ReadOtherType(TslBaseHUD))
					return;

				TslBaseHUD.EnumAllWidget([&](std::wstring Key, NativePtr<UUserWidget> WidgetPtr) -> bool {
					if (WidgetPtr.GetHash() != "MinimapOriginalType_C"h)
						return true;

					UMinimapOriginalType_C Minimap;
					if (!WidgetPtr.ReadOtherType(Minimap))
						return true;

					bRadarExtended = (Minimap.SelectMinimapSizeIndex > 0);
					return false;
					});

				DefaultFOV = PlayerController.DefaultFOV;

				if (PlayerController.Character)
					MyPawnPtr = (uintptr_t)PlayerController.Character;
				else if (CachedMyTslCharacterPtr)
					MyPawnPtr = (uintptr_t)CachedMyTslCharacterPtr;
				else if (PlayerController.SpectatorPawn)
					MyPawnPtr = (uintptr_t)PlayerController.SpectatorPawn;
				else
					MyPawnPtr = (uintptr_t)PlayerController.Pawn;
				CachedMyTslCharacterPtr = 0;

				UPlayerInput PlayerInput;
				if (!PlayerController.PlayerInput.Read(PlayerInput))
					return;

				FInputAxisProperties InputAxisProperties;
				if (PlayerInput.AxisProperties.GetValue(KeyMouseX, InputAxisProperties))
					MouseXSensitivity = InputAxisProperties.Sensitivity;
				if (PlayerInput.AxisProperties.GetValue(KeyMouseY, InputAxisProperties))
					MouseYSensitivity = InputAxisProperties.Sensitivity;

				APlayerCameraManager PlayerCameraManager;
				if (!PlayerController.PlayerCameraManager.Read(PlayerCameraManager))
					return;

				MyInfo.Location = CameraLocation = PlayerCameraManager.CameraCache_POV_Location;
				CameraRotation = PlayerCameraManager.CameraCache_POV_Rotation;
				CameraRotationMatrix = PlayerCameraManager.CameraCache_POV_Rotation.GetMatrix();
				CameraFOV = PlayerCameraManager.CameraCache_POV_FOV;
			}();
			////////////////////////////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////////
			const bool bLobby = (MyPawnPtr.GetHash() == "DefaultPawn"h);
			const bool bEnterGame = (!bLobby && bPrevLobby);
			bPrevLobby = bLobby;
			status += bLobby ? (std::string)"Lobby\n"e : "InGame\n"e;

			if (bLobby) {
				EnemyInfoMap.clear();
				LockAimbotTargetPtr = 0;
				IsFPPOnly = true;
				return;
			}

			if (IsNearlyZero(CameraFOV))
				return;
			if (IsNearlyZero(DefaultFOV))
				DefaultFOV = 90.0f;

			const float FOVRatio = DefaultFOV / CameraFOV;
			auto GetMouseXY = [&](FRotator RotationInput) {
				RotationInput.Clamp();
				return POINT{
					LONG(RotationInput.Yaw / MouseXSensitivity * 0.4f * FOVRatio),
					LONG(-RotationInput.Pitch / MouseYSensitivity * 0.4f * FOVRatio) };
			};

			const float AimbotCircleSize = tanf(ConvertToRadians(AimbotFOV)) * render.Height * powf(1.5f, log2f(FOVRatio));
			const float SilentCircleSize = tanf(ConvertToRadians(SilentFOV)) * render.Height * powf(1.5f, log2f(FOVRatio));
			float AimbotDistant = bAimbot ? AimbotCircleSize : SilentCircleSize;
			float ClosestDistant = render.Height / 2.0f;

			if (GetCharacterInfo((uintptr_t)MyPawnPtr, MyInfo))
				CachedMyTslCharacterPtr = (uintptr_t)MyPawnPtr;

			const float EjectDamage = EjectDamageCurve.Eval(MyInfo.GetSpeedXY(), 0.0f) / 1000.0f;
			if (bGameForeground && MyInfo.GetSpeedXYPerHour() < 200.0f && MyInfo.IsInVehicle && EjectDamage >= MyInfo.Health)
				bDeadByEject = true;

			if (CachedMyTslCharacterPtr) {
				if (!MyInfo.IsFPP)
					IsFPPOnly = false;
				status += (std::string)"Playing\n"e;
				if (MyInfo.IsWeaponed)
					status += (std::string)"Weaponed\n"e;
				if (MyInfo.IsReloading)
					status += (std::string)"Reloading\n"e;
				if (!MyInfo.IsWeaponReady)
					status += (std::string)"WeaponNotReady\n"e;
				status += MyInfo.IsFPP ? (std::string)"FPP\n"e : "TPP\n"e;
				status += IsFPPOnly ? (std::string)"FPP Only\n"e : "TPP Allowed\n"e;
				if (MyInfo.IsScoping)
					status += (std::string)"Zero : "e + std::to_string((unsigned)MyInfo.ZeroingDistance) + (std::string)"M\n"e;

				status += (std::string)"Speed : "e + std::to_string(unsigned(MyInfo.GetSpeedXYPerHour())) + (std::string)"\n"e;
				status += (std::string)"Health : "e + std::to_string(unsigned(MyInfo.Health * 100.0f)) + (std::string)"\n"e;
				status += (std::string)"EjectDamage : "e + std::to_string(unsigned(EjectDamage * 100.0f)) + (std::string)"\n"e;
				status += (std::string)"DeadByEject : "e + std::to_string(bDeadByEject) + (std::string)"\n"e;
			}
			else
				status += (std::string)"Spectating\n"e;

			status += (std::string)"SyncUser : \n"e;
			for (auto& UserName : UserInfo.GetSyncUserList())
				status += UserName + (std::string)"\n"e;

			if (!render.bKeyPushing[VK_MBUTTON])
				LockAimbotTargetPtr = 0;
			if (!render.bKeyPushing[VK_OEM_3])
				LockClosestTargetPtr = 0;

			CharacterInfo DummyInfo;
			if (!GetCharacterInfo(LockAimbotTargetPtr, DummyInfo))
				LockAimbotTargetPtr = 0;
			if (!GetCharacterInfo(LockClosestTargetPtr, DummyInfo))
				LockClosestTargetPtr = 0;

			NativePtr<ATslCharacter> AimbotTargetPtr = 0;
			NativePtr<ATslCharacter> ClosestTargetPtr = 0;

			auto ProcessTslCharacter = [&](uint64_t ActorPtr) {
				if (MyPawnPtr == ActorPtr && !bDebug)
					return;

				CharacterInfo Info;
				if (!GetCharacterInfo((uintptr_t)ActorPtr, Info))
					return;

				if (Info.Distance > 1500.0f)
					return;

				//DrawRadar
				[&] {
					if (!bRadar)
						return;

					if (Info.State == CharacterState::Dead)
						return;

					const float SpeedPerHour = MyInfo.GetSpeedXYPerHour();
					float RadarDistance =
						SpeedPerHour < 30.0f ? 200.0f :
						SpeedPerHour < 70.0f ? 250.0f :
						SpeedPerHour < 95.0f ? 300.0f :
						400.0f;

					if (bRadarExtended)
						RadarDistance *= RadarSizeLarge_FHD / RadarSizeNormal_FHD;

					if (RadarDistance != SavedRadarDistance) {
						LastRadarDistanceUpdateTime = WorldTimeSeconds;
						SavedRadarDistance = RadarDistance;
					}

					if (WorldTimeSeconds > LastRadarDistanceUpdateTime + 0.1f)
						LastRadarDistance = SavedRadarDistance;

					const ImVec2 RadarSize = bRadarExtended ? RadarSizeLarge : RadarSizeNormal;
					const ImVec2 RadarFrom = bRadarExtended ? RadarFromLarge : RadarFromNormal;
					const FVector RadarPos = (Info.Location - MyInfo.Location) * 0.01f;
					const FVector RadarScreenPos = {
						((1.0f + RadarPos.X / LastRadarDistance) * RadarSize.x / 2.0f) * render.Width,
						((1.0f + RadarPos.Y / LastRadarDistance) * RadarSize.y / 2.0f) * render.Height,
						0.0f
					};

					//GetColor
					ImColor Color = [&]()->ImColor {
						if (Info.Team == MyInfo.Team || Info.IsWhiteListed)
							return Render::COLOR_GREEN;
						if (Info.Health <= 0.0f)
							return Render::COLOR_GRAY;
						if (Info.IsFocusingMe)
							return Render::COLOR_RED;
						if (Info.IsAI || Info.IsDisconnected)
							return Render::COLOR_TEAL;
						if (Info.IsInVehicle)
							return Render::COLOR_BLUE;
						if (Info.IsBlackListed)
							return Render::COLOR_BLACK;
						if (Info.Team < 51)
							return TeamColors[Info.Team];

						return Render::COLOR_WHITE;
					}();

					ImGui::SetNextWindowPos({ RadarFrom.x * render.Width, RadarFrom.y * render.Height });
					ImGui::SetNextWindowSize({ RadarSize.x * render.Width, RadarSize.y * render.Height });
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
					ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
					ImGui::PushStyleColor(ImGuiCol_WindowBg, render.COLOR_CLEAR);
					ImGui::Begin("Radar"e, 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);
					ImGui::PopStyleColor();
					ImGui::PopStyleVar();
					ImGui::PopStyleVar();

					if (bDebug) {
						const ImVec2 WindowSize = ImGui::GetWindowSize();
						render.DrawLine({ WindowSize.x / 2.0f, 0.0f, 0.0f }, { WindowSize.x / 2.0f, WindowSize.y, 0.0f }, render.COLOR_RED);
						render.DrawLine({ 0.0f, WindowSize.y / 2.0f, 0.0f }, { WindowSize.x, WindowSize.y / 2.0f, 0.0f }, render.COLOR_RED);
					}

					constexpr float Size = 4.0f;

					if (Info.IsWeaponed && Info.AimOffsets.Length() > 0.0f) {
						FVector GunDir = FRotator(0.0f, Info.AimOffsets.Yaw, 0.0f).GetUnitVector();

						float Degree90 = ConvertToRadians(90.0f);
						FVector Dir1 = {
							GunDir.X * cosf(Degree90) - GunDir.Y * sinf(Degree90),
							GunDir.X * sinf(Degree90) + GunDir.Y * cosf(Degree90),
							0.0f };

						FVector Dir2 = {
							GunDir.X * cosf(-Degree90) - GunDir.Y * sinf(-Degree90),
							GunDir.X * sinf(-Degree90) + GunDir.Y * cosf(-Degree90),
							0.0f };

						FVector p1 = RadarScreenPos + GunDir * (Size + 1.0f) * 2.0f;
						FVector p2 = RadarScreenPos + Dir1 * (Size + 1.0f);
						FVector p3 = RadarScreenPos + Dir2 * (Size + 1.0f);
						render.DrawTriangle(p1, p2, p3, render.COLOR_BLACK);
						render.DrawCircle(RadarScreenPos, Size + 1.0f, render.COLOR_BLACK);

						p1 = RadarScreenPos + GunDir * Size * 2.0f;
						p2 = RadarScreenPos + Dir1 * Size;
						p3 = RadarScreenPos + Dir2 * Size;
						render.DrawTriangleFilled(p1, p2, p3, Color);
						render.DrawCircleFilled(RadarScreenPos, Size, Color);

						if (Info.TimeAfterShot < FiringTime)
							render.DrawLine(RadarScreenPos, RadarScreenPos + Info.LastFiringRot.GetUnitVector() * 500.0f, Color);
					}
					else {
						render.DrawCircle(RadarScreenPos, Size + 1.0f, render.COLOR_BLACK);
						render.DrawCircleFilled(RadarScreenPos, Size, Color);
					}

					ImGui::End();
				}();

				//ClosestEnemy
				[&] {
					if (Info.State == CharacterState::Dead)
						return;
					if (Info.IsAI)
						return;

					FVector AimPoint2D = WorldToScreen(Info.AimPoint);
					if (AimPoint2D.Z < 0.0f)
						return;

					AimPoint2D.Z = 0.0f;
					FVector Center2D = { render.Width / 2.0f, render.Height / 2.0f, 0 };

					float DistanceFromCenter = Center2D.Distance(AimPoint2D);

					if (DistanceFromCenter > ClosestDistant)
						return;

					ClosestTargetPtr = (uintptr_t)ActorPtr;
					ClosestDistant = DistanceFromCenter;
				}();

				bool IsInCircle = false;
				//Aimbot
				[&] {
					if (!MyInfo.IsWeaponed)
						return;
					if (!Info.IsVisible)
						return;
					if (!bTeamKill && (Info.Team == MyInfo.Team || Info.IsWhiteListed))
						return;
					if (!Info.IsLocked) {
						if (Info.State == CharacterState::Dead)
							return;
						if (Info.State == CharacterState::Groggy && !render.bKeyPushing[VK_CONTROL])
							return;
					}

					FVector AimPoint2D = WorldToScreen(Info.AimPoint);
					if (AimPoint2D.Z < 0.0f)
						return;

					AimPoint2D.Z = 0.0f;
					FVector Center2D = { render.Width / 2.0f, render.Height / 2.0f, 0 };

					float DistanceFromCenter = Center2D.Distance(AimPoint2D);
					if (DistanceFromCenter < AimbotCircleSize)
						IsInCircle = true;

					if (DistanceFromCenter > AimbotDistant)
						return;

					AimbotTargetPtr = (uintptr_t)ActorPtr;
					AimbotDistant = DistanceFromCenter;
				}();

				//DrawCharacter
				[&] {
					if (!bPlayer)
						return;

					if (Info.State == CharacterState::Dead)
						return;

					//GetColor
					ImColor Color = [&]()->ImColor {
						if (ActorPtr == LockClosestTargetPtr)
							return Render::COLOR_PURPLE;
						if (ActorPtr == LockAimbotTargetPtr)
							return Render::COLOR_PURPLE;
						if (Info.Team == MyInfo.Team || Info.IsWhiteListed)
							return Render::COLOR_GREEN;
						if (Info.Health <= 0.0f)
							return Render::COLOR_GRAY;
						if (Info.IsFocusingMe)
							return Render::COLOR_RED;
						if (Info.IsAI || Info.IsDisconnected)
							return Render::COLOR_TEAL;
						if (IsInCircle)
							return Render::COLOR_YELLOW;
						if (Info.IsInVehicle)
							return Render::COLOR_BLUE;
						if (Info.IsBlackListed)
							return Render::COLOR_BLACK;
						if (Info.Team < 51)
							return TeamColors[Info.Team];

						return Render::COLOR_WHITE;
					}();

					//Draw Skeleton
					if (ESP_PlayerSetting.bSkeleton) {
						for (auto DrawPair : GetDrawPairArray())
							render.DrawLine(Info.BonesScreenPos[DrawPair.first], Info.BonesScreenPos[DrawPair.second], Color);
					}

					//Draw HealthBar
					FVector HealthBarPos = Info.BonesPos[neck_01];
					HealthBarPos.Z += 35.0f;
					FVector HealthBarScreenPos = WorldToScreen(HealthBarPos);
					HealthBarScreenPos.Y -= 5.0f;

					float HealthBarScreenLengthY = 0.0f;
					if (ESP_PlayerSetting.bHealth) {
						const float CameraDistance = CameraLocation.Distance(HealthBarPos) / 100.0f;
						if (Info.Health > 0.0)
							HealthBarScreenLengthY = DrawRatioBox(HealthBarScreenPos, CameraDistance, 0.7f,
								Info.Health, Render::COLOR_GREEN, Render::COLOR_RED, Render::COLOR_BLACK).y;
						else if (Info.GroggyHealth > 0.0)
							HealthBarScreenLengthY = DrawRatioBox(HealthBarScreenPos, CameraDistance, 0.7f,
								Info.GroggyHealth, Render::COLOR_RED, Render::COLOR_GRAY, Render::COLOR_BLACK).y;
					}

					//Draw CharacterInfo
					std::string PlayerInfo;
					std::string Line;
					bool bShortNick = ESP_PlayerSetting.bShortNick && !render.bKeyPushing[VK_MBUTTON];

					if (ESP_PlayerSetting.bNickName) {
						if (!bShortNick)
							Line += Info.PlayerName;
						else if (Info.IsAI)
							Line += (std::string)"Bot"e;
						else if (Info.PlayerName.size() > MaxShortNickLen)
							Line += Info.PlayerName.substr(0, MaxShortNickLen) + (std::string)"..."e;
						else
							Line += Info.PlayerName;
					}

					if (ESP_PlayerSetting.bLevel) {
						if (!Info.IsAI && Info.Level != -1 && !bShortNick) {
							if (Line.size())
								Line += (std::string)" "e;
							Line += std::to_string(Info.Level);
						}
					}

					if (ESP_PlayerSetting.bTeam) {
						if (Info.Team < 200 && Info.Team != MyInfo.Team) {
							if (Line.size())
								Line += (std::string)" "e;
							Line += std::to_string(Info.Team);
						}
					}

					if (ESP_PlayerSetting.bRankInfo && Info.PlayerName.size() && !Info.IsAI) {
						std::map<unsigned, tUserInfo>& UserInfoMap = *[&] {
							const bool bSolo = !(Info.Team < 200);
							if (ESP_PlayerSetting.bKakao && !bSolo && !IsFPPOnly)
								return &UserInfo.InfoKakaoSquad;
							if (IsFPPOnly && !bSolo)
								return &UserInfo.InfoSteamSquadFPP;
							if (!IsFPPOnly && bSolo)
								return &UserInfo.InfoSteamSolo;
							if (!IsFPPOnly && !bSolo)
								return &UserInfo.InfoSteamSquad;
							return &UserInfo.InfoEmpty;
						}();

						UserInfo.AddUser(Info.PlayerName, ESP_PlayerSetting.bKakao);

						const std::string InfoStr = [&]() -> std::string {
							const unsigned NameHash = CompileTime::StrHash(Info.PlayerName);
							if (!UserInfoMap.contains(NameHash))
								return "?"e;

							auto& UserInfo = UserInfoMap[NameHash];
							if (!UserInfo.bExist)
								return "0"e;

							const unsigned Factor = bShortNick ? 100 : 1;
							const unsigned RankPoint = unsigned(UserInfo.rankPoint / Factor);

							return bShortNick ?
								sformat((const char*)"{} {:.1f}"e, RankPoint, UserInfo.AvgKills) :
								sformat((const char*)"{} {:.1f} {:.0f}"e, RankPoint, UserInfo.AvgKills, UserInfo.AvgDmg);
						}();

						Line += std::string("("e) + InfoStr + std::string(")"e);
					}

					if (Line.size()) {
						PlayerInfo += Line;
						PlayerInfo += (std::string)"\n"e;
						Line = {};
					}

					if (ESP_PlayerSetting.bWeapon) {
						Line += Info.GetWeaponInfoStr();
						if (!bShortNick) {
							if (Info.Weapon1Name.size())
								Line += (std::string)" "e + Info.Weapon1Name;
							if (Info.Weapon2Name.size())
								Line += (std::string)" "e + Info.Weapon2Name;
						}
					}

					if (Line.size()) {
						PlayerInfo += Line;
						PlayerInfo += (std::string)"\n"e;
						Line = {};
					}

					if (ESP_PlayerSetting.bDistance) {
						Line += std::to_string((int)Info.Distance);
						Line += (std::string)"M"e;
					}

					if (ESP_PlayerSetting.bKill && Info.NumKills) {
						if (Line.size())
							Line += (std::string)" "e;
						Line += std::to_string(Info.NumKills) + (std::string)"K"e;
					}

					if (ESP_PlayerSetting.bSpectatedCount && Info.SpectatedCount) {
						if (Line.size())
							Line += (std::string)" "e;
						Line += std::to_string(Info.SpectatedCount) + (std::string)"W"e;
					}

					if (ESP_PlayerSetting.bDamage && Info.Damage && !bShortNick) {
						if (Line.size())
							Line += (std::string)" "e;
						Line += std::to_string((int)Info.Damage) + (std::string)"D"e;
					}

					if (Line.size()) {
						PlayerInfo += Line;
						PlayerInfo += (std::string)"\n"e;
						Line = {};
					}

					DrawString(
						{ HealthBarScreenPos.X, HealthBarScreenPos.Y - HealthBarScreenLengthY - GetTextHeight(PlayerInfo) / 2.0f, HealthBarScreenPos.Z },
						PlayerInfo, Color, true);
				}();
			};

			//Actor loop
			for (const auto& ActorPtr : Actors.GetVector()) {
				TSet<NativePtr<UActorComponent>> OwnedComponents;
				FVector ActorVelocity;
				FVector ActorLocation;
				FVector ActorLocationScreen;
				float DistanceToActor = 0.0f;
				unsigned ActorNameHash = 0;
				bool bAircraftCarePackage = false;

				auto GetValidActorInfo = [&]() {
					AActor Actor;
					if (!ActorPtr.Read(Actor))
						return false;

					OwnedComponents = Actor.OwnedComponents;

					USceneComponent RootComponent;
					if (!Actor.RootComponent.Read(RootComponent))
						return false;

					ActorVelocity = RootComponent.ComponentVelocity;
					ActorLocation = RootComponent.ComponentToWorld.Translation;
					ActorLocationScreen = WorldToScreen(ActorLocation);
					DistanceToActor = MyInfo.Location.Distance(ActorLocation) / 100.0f;

					if (DistanceToActor >= 3000)
						return false;
					if (nRange != 1000 && DistanceToActor >= nRange)
						return false;

					USceneComponent AttachParent;
					//LocalPawn is vehicle(Localplayer is in vehicle)
					if (RootComponent.AttachParent.Read(AttachParent) && AttachParent.Owner == MyPawnPtr)
						return false;

					const std::string ActorName = Actor.GetFName().GetName();
					if (ActorName.empty())
						return false;

					ActorNameHash = CompileTime::StrHash(ActorName);
					bAircraftCarePackage = (ActorName.substr(0, sizeof("AircraftCarePackage"e) - 1) == (std::string)"AircraftCarePackage"e);

					if (bDebug) {
						FVector v2_DebugLoc = ActorLocationScreen;
						v2_DebugLoc.Y += 15.0;
						DrawString(v2_DebugLoc, ActorName, Render::COLOR_WHITE, false);
					}

					return true;
				};
				if (!GetValidActorInfo())
					continue;

				//DrawAircraft
				[&] {
					if (!bAircraftCarePackage)
						return;

					auto Output = sformat((const char*)"{}\n{}M"e, (const char*)"Aircraft"e, (unsigned)DistanceToActor);
					DrawString(ActorLocationScreen, Output, Render::COLOR_TEAL, false);
				}();

				//DrawObject
				[&] {
					auto ObjectInfo = GetObjectInfo(ActorNameHash);
					auto& ObjectName = ObjectInfo.Name;
					if (!ObjectName[0])
						return;

					if (!ObjectInfo.IsLong && DistanceToActor > 300.0f)
						return;

					auto Output = sformat((const char*)"{}\n{}M"e, ObjectName.data(), (unsigned)DistanceToActor);
					DrawString(ActorLocationScreen, Output, ObjectInfo.bFrendly ? Render::COLOR_TEAL : Render::COLOR_RED, false);
				}();

				//DrawVehicle
				[&] {
					if (!bVehicle || bFighterMode)
						return;

					if (ActorNameHash == "BP_VehicleDrop_BRDM_C"h) {
						auto Output = sformat((const char*)"BRDM\n{}M"e, (unsigned)DistanceToActor);
						DrawString(ActorLocationScreen, Output, Render::COLOR_TEAL, false);
						return;
					}

					auto VehicleInfo = GetVehicleInfo(ActorNameHash);
					auto& VehicleName = VehicleInfo.Name;
					if (!VehicleName[0])
						return;

					float Health = 100.0f;
					float HealthMax = 100.0f;
					float Fuel = 100.0f;
					float FuelMax = 100.0f;

					switch (VehicleInfo.Type1) {
					case VehicleType1::Wheeled:
					{
						ATslWheeledVehicle WheeledVehicle;
						if (!ActorPtr.ReadOtherType(WheeledVehicle))
							break;

						UTslVehicleCommonComponent VehicleComponent;
						if (!WheeledVehicle.VehicleCommonComponent.Read(VehicleComponent))
							break;

						Health = VehicleComponent.Health;
						HealthMax = VehicleComponent.HealthMax;
						Fuel = VehicleComponent.Fuel;
						FuelMax = VehicleComponent.FuelMax;
						break;
					}
					case VehicleType1::Floating:
					{
						ATslFloatingVehicle WheeledVehicle;
						if (!ActorPtr.ReadOtherType(WheeledVehicle))
							break;

						UTslVehicleCommonComponent VehicleComponent;
						if (!WheeledVehicle.VehicleCommonComponent.Read(VehicleComponent))
							break;

						Health = VehicleComponent.Health;
						HealthMax = VehicleComponent.HealthMax;
						Fuel = VehicleComponent.Fuel;
						FuelMax = VehicleComponent.FuelMax;
						break;
					}
					}

					if (ActorNameHash == "BP_LootTruck_C"h && Health <= 0.0f)
						return;

					bool IsDestructible = (VehicleInfo.Type2 == VehicleType2::Destructible);

					ImColor Color = Render::COLOR_BLUE;
					if (VehicleInfo.Type3 == VehicleType3::Special)
						Color = Render::COLOR_TEAL;
					if (Health <= 0.0f || Fuel <= 0.0f)
						Color = Render::COLOR_GRAY;

					auto Output = sformat((const char*)"{}\n{}M"e, VehicleName.data(), (unsigned)DistanceToActor);
					DrawString(ActorLocationScreen, Output, Color, false);

					//Draw vehicle health, fuel
					[&] {
						if (!IsDestructible)
							return;

						FVector VehicleBarScreenPos = ActorLocationScreen;
						VehicleBarScreenPos.Y += GetTextHeight(Output) / 2.0f + 4.0f;
						const float CameraDistance = CameraLocation.Distance(ActorLocation) / 100.0f;
						if (Health <= 0.0f)
							return;

						const float HealthBarScreenLengthY = DrawRatioBox(VehicleBarScreenPos, CameraDistance, 1.0f,
							Health / HealthMax, Render::COLOR_GREEN, Render::COLOR_RED, Render::COLOR_BLACK).y;
						VehicleBarScreenPos.Y += HealthBarScreenLengthY - 1.0f;
						DrawRatioBox(VehicleBarScreenPos, CameraDistance, 1.0f,
							Fuel / FuelMax, Render::COLOR_BLUE, Render::COLOR_GRAY, Render::COLOR_BLACK).y;
					}();
				}();

				//DrawPackage
				[&] {
					if (!bBox || bFighterMode)
						return;

					auto PackageInfo = GetPackageInfo(ActorNameHash);
					auto& PackageName = PackageInfo.Name;
					if (!PackageName[0])
						return;

					if (PackageInfo.IsSmall && DistanceToActor > 300.0f)
						return;

					auto Output = PackageInfo.IsSmall ?
						std::string(PackageName.data()) :
						sformat((const char*)"{}\n{}M"e, PackageName.data(), (unsigned)DistanceToActor);

					DrawString(ActorLocationScreen, Output, Render::COLOR_TEAL, false);

					//DrawBoxContents
					[&] {
						if (!render.bKeyPushing[VK_MBUTTON] || nItem == 0)
							return;

						AItemPackage ItemPackage;
						if (!ActorPtr.ReadOtherType(ItemPackage))
							return;

						const float TextHeight = GetTextHeight((const char*)""e);
						FVector PackageLocationScreen = ActorLocationScreen;
						PackageLocationScreen.Y += (GetTextHeight(Output) + TextHeight) / 2.0f;

						for (const auto& ItemPtr : ItemPackage.Items.GetVector()) {
							if (!DrawItem(ItemPtr, PackageLocationScreen))
								continue;
							PackageLocationScreen.Y += TextHeight - 1.0f;
						}
					}();
				}();

				//DrawDropptedItem
				[&] {
					if (nItem == 0 || bFighterMode || ActorNameHash != "DroppedItem"h)
						return;

					ADroppedItem DroppedItem;
					if (!ActorPtr.ReadOtherType(DroppedItem))
						return;

					DrawItem((uintptr_t)DroppedItem.Item, ActorLocationScreen);
				}();

				//DrawDroppedItemGroup
				[&] {
					if (nItem == 0 || bFighterMode || ActorNameHash != "DroppedItemGroup"h)
						return;

					for (const auto& Element : OwnedComponents.GetVector()) {
						UDroppedItemInteractionComponent ItemComponent;
						if (!Element.Value.ReadOtherType(ItemComponent))
							continue;

						unsigned ItemComponentHash = ItemComponent.GetFName().GetHash();
						if (!ItemComponentHash)
							continue;

						if (ItemComponentHash != "DroppedItemInteractionComponent"h && ItemComponentHash != "DestructibleItemInteractionComponent"h)
							continue;

						FVector ItemLocationScreen = WorldToScreen(ItemComponent.ComponentToWorld.Translation);
						DrawItem(ItemComponent.Item, ItemLocationScreen);
					}
				}();

				ProcessTslCharacter(ActorPtr);
			}

			float CustomTimeDilation = 1.0f;

			if (IsPenetrateOn()) {
				FVector Direction = CameraRotation.GetUnitVector();
				MyInfo.AimLocation = MyInfo.BonesPos[forehead] + Direction * 80.0f;

				ChangeRegOnBPInfo Info{};
				Info.changeXMM0_0 = true;
				Info.changeXMM0_1 = true;
				Info.XMM0.Float_0 = MyInfo.AimLocation.X;
				Info.XMM0.Float_1 = MyInfo.AimLocation.Y;
				dbvm.ChangeRegisterOnBP(GunLocScopeHookAddressPA1, Info);
				dbvm.ChangeRegisterOnBP(GunLocNoScopeHookAddressPA1, Info);

				Info = {};
				Info.changeRAX = true;
				Info.newRAX = *(int*)&MyInfo.AimLocation.Z;
				dbvm.ChangeRegisterOnBP(GunLocScopeHookAddressPA2, Info);
				dbvm.ChangeRegisterOnBP(GunLocNoScopeHookAddressPA2, Info);

				Info = {};
				Info.changeXMM0_0 = true;
				Info.changeXMM1_0 = true;
				Info.changeXMM2_0 = true;
				Info.XMM0.Float_0 = MyInfo.AimLocation.X;
				Info.XMM1.Float_0 = MyInfo.AimLocation.Y;
				Info.XMM2.Float_0 = MyInfo.AimLocation.Z;
				dbvm.ChangeRegisterOnBP(GunLocNearWallHookAddressPA, Info);
				IsNeedToHookGunLoc = true;
			}

			//ClosestTarget
			[&] {
				if (!LockClosestTargetPtr)
					LockClosestTargetPtr = ClosestTargetPtr;

				if (!render.bKeyPushing[VK_OEM_3])
					return;

				CharacterInfo ClosestTargetInfo;
				if (!GetCharacterInfo(LockClosestTargetPtr, ClosestTargetInfo))
					return;

				const std::string& Name = ClosestTargetInfo.PlayerName;
				const unsigned NameHash = CompileTime::StrHash(Name);

				if (Name.empty())
					return;

				if (render.bKeyPushed[VK_MBUTTON])
					UserInfo.Invalidate(Name, ESP_PlayerSetting.bKakao);

				if (render.bKeyPushed[VK_RBUTTON])
					OpenWebUserInfo(Name.c_str());

				if (render.bKeyPushed[VK_ADD])
					AddUserToList(BlackList, BlackListFile, Name.c_str());

				if (render.bKeyPushed[VK_SUBTRACT])
					RemoveUserFromList(BlackList, BlackListFile, Name.c_str());
			}();

			//AutoClick
			const bool IsAutoClicking = [&] {
				if (!IsAutoClickOn)
					return false;

				if (!MyInfo.IsWeaponed)
					return false;

				if (!MyInfo.IsProperForAutoClick)
					return false;

				if (MyInfo.WeaponType == tWeaponType::SR && MyInfo.IsScoping) {
					if (!MyInfo.IsWeaponReady)
						return false;
					if (MyInfo.Ammo < 1)
						return false;
				}

				CURSORINFO CursorInfo = { .cbSize = sizeof(CURSORINFO) };
				GetCursorInfo(&CursorInfo);
				return CursorInfo.flags == 0;
			}();

			if (IsAutoClicking)
				AutoClick(hGameWnd);

			//Aimbot
			[&] {
				if (!MyInfo.IsWeaponed)
					return;

				if (!AimbotTargetPtr)
					return;

				if (!LockAimbotTargetPtr)
					LockAimbotTargetPtr = AimbotTargetPtr;

				CharacterInfo AimbotTargetInfo;
				if (!GetCharacterInfo(LockAimbotTargetPtr, AimbotTargetInfo))
					return;

				const FVector TargetPos = AimbotTargetInfo.AimPoint;
				const FVector TargetVelocity = AimbotTargetInfo.Velocity;

				auto Result = GetBulletDropAndTravelTime(
					MyInfo.AimLocation,
					MyInfo.AimRotation,
					TargetPos,
					MyInfo.ZeroingDistance,
					MyInfo.BulletDropAdd,
					MyInfo.InitialSpeed,
					MyInfo.Gravity,
					BallisticDragScale,
					BallisticDropScale,
					MyInfo.BDS,
					MyInfo.SimulationSubstepTime,
					MyInfo.VDragCoefficient,
					MyInfo.BallisticCurve);

				float BulletDrop = Result.first;
				float TravelTime = Result.second;

				FVector PredictedPos = FVector(TargetPos.X, TargetPos.Y, TargetPos.Z + BulletDrop) + TargetVelocity * (TravelTime / CustomTimeDilation);
				FVector TargetScreenPos = WorldToScreen(TargetPos);
				FVector AimScreenPos = WorldToScreen(PredictedPos);

				const float LineLen = std::clamp(AimScreenPos.Y - WorldToScreen({ TargetPos.X, TargetPos.Y, TargetPos.Z + 10.0f }).Y, 4.0f, 8.0f);
				render.DrawLine(TargetScreenPos, AimScreenPos, Render::COLOR_RED, 2.0f);
				render.DrawX(AimScreenPos, LineLen, Render::COLOR_RED, 2.0f);

				auto AImbot_MouseMove_Old = [&] {
					TimeDeltaAcc += render.TimeDelta;
					if (WorldTimeSeconds == LastAimUpdateTime)
						return;

					LastAimUpdateTime = WorldTimeSeconds;
					const float TimeDelta = TimeDeltaAcc;
					TimeDeltaAcc = 0.0f;

					const FVector LocTarget = ::WorldToScreen(PredictedPos, CameraRotationMatrix, CameraLocation, CameraFOV, 1.0f, 1.0f);
					const float DistanceToTarget = CameraLocation.Distance(PredictedPos) / 100.0f;
					const FVector GunCenterPos = CameraLocation + MyInfo.AimRotation.GetUnitVector() * DistanceToTarget;
					const FVector LocCenter = ::WorldToScreen(GunCenterPos, CameraRotationMatrix, CameraLocation, CameraFOV, 1.0f, 1.0f);

					const float MouseX = RemainMouseX + AimSpeedX * (LocTarget.X - LocCenter.X) * TimeDelta * 100000.0f;
					const float MouseY = RemainMouseY + AimSpeedY * (LocTarget.Y - LocCenter.Y) * TimeDelta * 100000.0f;
					RemainMouseX = MouseX - truncf(MouseX);
					RemainMouseY = MouseY - truncf(MouseY);
					MoveMouse(hGameWnd, { (int)MouseX, (int)MouseY });
				};

				auto AImbot_MouseMove = [&](bool bMoveX, bool bMoveY) {
					TimeDeltaAcc += render.TimeDelta;
					if (WorldTimeSeconds == LastAimUpdateTime)
						return;

					LastAimUpdateTime = WorldTimeSeconds;
					const float TimeDelta = TimeDeltaAcc;
					TimeDeltaAcc = 0.0f;

					FRotator RotationInput = (PredictedPos - CameraLocation).GetDirectionRotator() - MyInfo.AimRotation;
					RotationInput.Clamp();
					POINT MaxXY = GetMouseXY(RotationInput * AimSpeedMaxFactor);
					if (!bMoveX) MaxXY.x = 0;
					if (!bMoveY) MaxXY.y = 0;
					if (MaxXY.x == 0 && MaxXY.y == 0) {
						RemainMouseX = RemainMouseY = 0.0f;
						return;
					}

					FVector FMouseXY = { (float)MaxXY.x, (float)MaxXY.y, 0.0f };
					FMouseXY.Normalize();

					const float MouseX = RemainMouseX + std::clamp(AimSpeedX * TimeDelta * FMouseXY.X, -(float)abs(MaxXY.x), (float)abs(MaxXY.x));
					const float MouseY = RemainMouseY + std::clamp(AimSpeedY * TimeDelta * FMouseXY.Y, -(float)abs(MaxXY.y), (float)abs(MaxXY.y));
					RemainMouseX = MouseX - truncf(MouseX);
					RemainMouseY = MouseY - truncf(MouseY);
					MoveMouse(hGameWnd, { (int)MouseX, (int)MouseY });
				};

				if (!render.bKeyPushing[VK_MBUTTON])
					return;

				if (hGameWnd != hForeWnd)
					return;

				[&] {
					if (!bAimbot)
						return;

					if (MyInfo.WeaponType == tWeaponType::SR && MyInfo.IsScoping) {
						if (!MyInfo.IsWeaponReady)
							return;
						if (MyInfo.Ammo < 1)
							return;
					}

					if (MyInfo.IsReloading)
						return;

					const bool bMoveY = [&] {
						if (!MyInfo.IsScoping)
							return true;

						switch (CompileTime::StrHash(MyInfo.WeaponName)) {
						case "R45"h:
						case "Deagle"h:
						case "R1895"h:
							if (MyInfo.TimeAfterShot < 0.15f)
								return false;
						case "P1911"h:
						case "P92"h:
							if (MyInfo.TimeAfterShot < 0.07f)
								return false;
						}

						if (!IsAutoClicking && !MyInfo.IsAutoFiring && MyInfo.TimeAfterShot < 0.1f)
							return false;

						return true;
					}();

					AImbot_MouseMove(true, bMoveY);
				}();

				if (bSilentAim && (bSilentAim_DangerousMode || MyInfo.IsScoping)) {
					FVector AimPoint2D = WorldToScreen(TargetPos);
					if (AimPoint2D.Z < 0.0f)
						return;

					AimPoint2D.Z = 0.0f;
					FVector Center2D = { render.Width / 2.0f, render.Height / 2.0f, 0 };

					float DistanceFromCenter = Center2D.Distance(AimPoint2D);
					if (DistanceFromCenter > SilentCircleSize)
						return;

					FVector RandedTargetPos = TargetPos;
					float radiousMax = [&] {
						if (render.bKeyPushing[VK_SHIFT])
							return RandSilentAimHead;
						switch (MyInfo.WeaponType) {
						case tWeaponType::SR:
						case tWeaponType::SG:
							return RandSilentAimHead;
						}
						return RandSilentAimBody;
					}();
					float radious = randf(0.0f, radiousMax);
					float angle1 = randf(0.0f, PI);
					float angle2 = randf(0.0f, PI * 2.0f);
					RandedTargetPos.X += radious * sinf(angle1) * cosf(angle2);
					RandedTargetPos.Y += radious * sinf(angle1) * sinf(angle2);
					RandedTargetPos.Z += radious * cosf(angle1);

					Result = GetBulletDropAndTravelTime(
						MyInfo.AimLocation,
						MyInfo.AimRotation,
						RandedTargetPos,
						FLT_MIN,
						MyInfo.BulletDropAdd,
						MyInfo.InitialSpeed,
						MyInfo.Gravity,
						BallisticDragScale,
						BallisticDropScale,
						MyInfo.BDS,
						MyInfo.SimulationSubstepTime,
						MyInfo.VDragCoefficient,
						MyInfo.BallisticCurve);

					BulletDrop = Result.first;
					TravelTime = Result.second;
					PredictedPos = FVector(RandedTargetPos.X, RandedTargetPos.Y, RandedTargetPos.Z + BulletDrop) 
						+ TargetVelocity * (TravelTime / CustomTimeDilation);

					FVector DirectionInput = PredictedPos - MyInfo.AimLocation;
					DirectionInput.Normalize();

					ChangeRegOnBPInfo Info{};
					Info.changeXMM6_0 = true;
					Info.changeXMM7_0 = true;
					Info.changeXMM8_0 = true;
					Info.XMM6.Float_0 = DirectionInput.X;
					Info.XMM7.Float_0 = DirectionInput.Y;
					Info.XMM8.Float_0 = DirectionInput.Z;
					dbvm.ChangeRegisterOnBP(AimHookAddressPA, Info);
					IsNeedToHookAim = true;
				}
			}();

			if (!render.bKeyPushing[VK_CAPITAL]) {
				EnemyFocusingMePtr = 0;
				bPushedCapsLock = false;
			}
			else if(!bPushedCapsLock) {
				bPushedCapsLock = true;

				switch (nCapsLockMode) {
				case 1:
					MoveMouse(hGameWnd, GetMouseXY({ 0.0f, 180.0f, 0.0f }));
					break;
				case 2:
					NativePtr<ATslCharacter> Ptr;

					for (auto Elem : EnemyInfoMap) {
						if (Elem.second.FocusTime > MinFocusTime) {
							if (Elem.first == EnemyFocusingMePtr)
								break;
							if (!Ptr)
								Ptr = Elem.first;
						}
						else if (Elem.first == EnemyFocusingMePtr)
							EnemyFocusingMePtr = 0;
					}

					CharacterInfo Info;
					if (!GetCharacterInfo(Ptr, Info))
						break;

					LockAimbotTargetPtr = Info.Ptr;
					EnemyFocusingMePtr = Info.Ptr;

					auto Result = GetBulletDropAndTravelTime(
						MyInfo.AimLocation,
						MyInfo.AimRotation,
						Info.BonesPos[forehead],
						MyInfo.ZeroingDistance,
						MyInfo.BulletDropAdd,
						MyInfo.InitialSpeed,
						MyInfo.Gravity,
						BallisticDragScale,
						BallisticDropScale,
						MyInfo.BDS,
						MyInfo.SimulationSubstepTime,
						MyInfo.VDragCoefficient,
						MyInfo.BallisticCurve);

					float BulletDrop = Result.first;
					float TravelTime = Result.second;

					FVector PredictedPos = FVector(Info.AimPoint.X, Info.AimPoint.Y, Info.AimPoint.Z + BulletDrop) + Info.Velocity * (TravelTime / CustomTimeDilation);
					FRotator RotationInput = (PredictedPos - CameraLocation).GetDirectionRotator() - MyInfo.AimRotation;
					RotationInput.Clamp();
					MoveMouse(hGameWnd, GetMouseXY(RotationInput));
					break;
				}
			}

			if (!IsNeedToHookAim)
				dbvm.RemoveChangeRegisterOnBP(AimHookAddressPA);
			if (!IsNeedToHookGunLoc) {
				dbvm.RemoveChangeRegisterOnBP(GunLocScopeHookAddressPA1);
				dbvm.RemoveChangeRegisterOnBP(GunLocScopeHookAddressPA2);
				dbvm.RemoveChangeRegisterOnBP(GunLocNoScopeHookAddressPA1);
				dbvm.RemoveChangeRegisterOnBP(GunLocNoScopeHookAddressPA2);
				dbvm.RemoveChangeRegisterOnBP(GunLocNearWallHookAddressPA);
			}

			if (MyInfo.SpectatedCount > 0)
				DrawSpectatedCount(MyInfo.SpectatedCount, Render::COLOR_RED);

			std::string Enemies;
			for (auto Elem : EnemyInfoMap) {
				if (Elem.second.FocusTime < MinFocusTime)
					continue;

				CharacterInfo Info;
				if (!GetCharacterInfo(Elem.first, Info))
					continue;

				Enemies += sformat((const char*)"{} : {} : {:.0f}M\n"e, Info.PlayerName, Info.GetWeaponInfoStr(), Info.Distance);
			}
			DrawEnemiesFocusingMe(Enemies.c_str(), Render::COLOR_RED);

			if (MyInfo.IsWeaponed) {
				if (bAimbot)
					render.DrawCircle({ render.Width / 2.0f, render.Height / 2.0f, 0.0f }, AimbotCircleSize, IsPenetrateOn() ? IM_COL32(255, 0, 0, 100) : IM_COL32(255, 255, 255, 100));
				if (bSilentAim)
					render.DrawCircle({ render.Width / 2.0f, render.Height / 2.0f, 0.0f }, SilentCircleSize, IM_COL32(255, 255, 0, 100));
			}
		};

		render.RenderArea(hGameWnd, Render::COLOR_CLEAR, [&] {
			if (!ExceptionHandler::TryExcept(FuncInRenderArea))
				printlog("Error : %X\n"e, ExceptionHandler::GetLastExceptionCode());
			});

#ifdef _WINDLL
		bBlockKey = bDeadByEject;
		Window.MessageLoop();
#endif
	}
}
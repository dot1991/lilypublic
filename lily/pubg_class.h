#pragma once
#include "common/defclass.h"
#include "pubg_struct.h"
#include "GObjects.h"

#include "info_weapon.h"
#include "info_item.h"

class UWorld;
class ULevel;
class UGameInstance;
class ULocalPlayer;
class AController;
class APlayerController;
class UPlayerInput;
class APlayerCameraManager;
class AActor;
class APlayerState;
class APawn;
class ACharacter;
class UTslSettings;
class ATslCharacter;
class ATslWheeledVehicle;
class ATslFloatingVehicle;
class UTslVehicleCommonComponent;
class UActorComponent;
class USceneComponent;
class UPrimitiveComponent;
class UCapsuleComponent;
class UMeshComponent;
class USkinnedMeshComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class USkeletalMeshSocket;
class UStaticMeshSocket;
class USkeletalMesh;
class UStaticMesh;
class UWeaponMeshComponent;
class USkeleton;
class ADroppedItem;
class UDroppedItemInteractionComponent;
class FItemTableRowBase;
class UItem;
class UVehicleRiderComponent;
class UWeaponProcessorComponent;
class ATslWeapon;
class ATslWeapon_Gun;
class ATslWeapon_Trajectory;
class ATslWeapon_Melee;
class ATslWeapon_Throwable;
class UAnimInstance;
class UTslAnimInstance;
class UWeaponTrajectoryData;
class AItemPackage;
class ATslPlayerState;
class UCurveVector;
class FWeaponAttachmentWeaponTagData;
class FItemTableRowAttachment;
class AHUD;

//Engine.World.CurrentLevel 
//Function TslGame.TslLivingThing.GetWorldTimeSeconds 
DefClass(UWorld, UObject,
	MemberAtOffset(EncryptedPtr<ULevel>, CurrentLevel, 0x140)
	MemberAtOffset(EncryptedPtr<UGameInstance>, GameInstance, 0x228)

	MemberAtOffset(float, TimeSeconds, 0x630)
	,
	constexpr static uintptr_t BASE = 0x8E5A180;
	static bool GetUWorld(UWorld& World);
)

DefClass(UGameInstance, UObject,
	MemberAtOffset(TArray<EncryptedPtr<ULocalPlayer>>, LocalPlayers, 0x58)
)

DefClass(ULocalPlayer, UObject,
	MemberAtOffset(EncryptedPtr<APlayerController>, PlayerController, 0x48)
)

//Engine.Controller.Character 
//Engine.Controller.Pawn 
DefClass(AController, UObject,
	MemberAtOffset(EncryptedPtr<ACharacter>, Character, 0x458)
	MemberAtOffset(EncryptedPtr<APawn>, Pawn, 0x448)
)

//Function Engine.PlayerInput.SetMouseSensitivity 
DefClass(UPlayerInput, UObject,
	MemberAtOffset(UNPACK(TMap<FName, FInputAxisProperties>), AxisProperties, 0x148)
)

//Engine.PlayerController.SpectatorPawn 
//Engine.PlayerController.PlayerCameraManager 
//Engine.PlayerController.PlayerInput 
DefClass(APlayerController, AController,
	MemberAtOffset(NativePtr<APawn>, SpectatorPawn, 0x748)
	MemberAtOffset(NativePtr<APlayerCameraManager>, PlayerCameraManager, 0x4b8)
	MemberAtOffset(NativePtr<UPlayerInput>, PlayerInput, 0x530)
	MemberAtOffset(NativePtr<AHUD>, MyHUD, 0x4b0)
)

//TslGame.TslPlayerController.DefaultFOV 
DefClass(ATslPlayerController, APlayerController,
	MemberAtOffset(float, DefaultFOV, 0xacc)
)

//Engine.PlayerCameraManager.CameraCache +
//Engine.CameraCacheEntry.POV +
//Engine.MinimalViewInfo.Fov 
//Engine.MinimalViewInfo.Rotation 
//Engine.MinimalViewInfo.Location 
DefClass(APlayerCameraManager, UObject,
	MemberAtOffset(float, CameraCache_POV_FOV, 0x1650 + 0x10 + 0x598)
	MemberAtOffset(FRotator, CameraCache_POV_Rotation, 0x1650 + 0x10 + 0x5a8)
	MemberAtOffset(FVector, CameraCache_POV_Location, 0x1650 + 0x10 + 0x588)
)

DefClass(ULevel, UObject,
	MemberAtOffset(EncryptedPtr<TArray<NativePtr<AActor>>>, Actors, 0x1C0)
)

//Engine.Actor.RootComponent 
//Engine.Actor.ReplicatedMovement 
DefClass(AActor, UObject,
	MemberAtOffset(EncryptedPtr<USceneComponent>, RootComponent, 0xf8)
	MemberAtOffset(FRepMovement, ReplicatedMovement, 0x78)

	MemberAtOffset(TSet<NativePtr<UActorComponent>>, OwnedComponents, 0x358)
	,
	FTransform ActorToWorld() const;
)

DefClass(APlayerState, UObject, )

//Engine.Pawn.PlayerState 
DefClass(APawn, AActor,
	MemberAtOffset(EncryptedPtr<APlayerState>, PlayerState, 0x430)
)

//Engine.Character.Mesh 
DefClass(ACharacter, APawn,
	MemberAtOffset(NativePtr<USkeletalMeshComponent>, Mesh, 0x4c0)
	MemberAtOffset(FVector, BaseTranslationOffset, 0x4cc)
)

//TslGame.TslCharacter.Health 
//TslGame.TslCharacter.HealthMax 
//TslGame.TslCharacter.GroggyHealth 
//TslGame.TslCharacter.GroggyHealthMax 
//TslGame.TslCharacter.CharacterName 
//TslGame.TslCharacter.LastTeamNum 
//TslGame.TslCharacter.VehicleRiderComponent 
//TslGame.TslCharacter.WeaponProcessor 
//TslGame.TslCharacter.SpectatedCount 
DefClass(ATslCharacter, ACharacter,
	MemberAtOffset(float, Health, 0x1190)
	MemberAtOffset(float, HealthMax, 0x1890)
	MemberAtOffset(float, GroggyHealth, 0x1924)
	MemberAtOffset(float, GroggyHealthMax, 0xe30)
	MemberAtOffset(FString, CharacterName, 0x1108)
	MemberAtOffset(int, LastTeamNum, 0x1050)
	MemberAtOffset(NativePtr<UVehicleRiderComponent>, VehicleRiderComponent, 0x1c40)
	MemberAtOffset(NativePtr<UWeaponProcessorComponent>, WeaponProcessor, 0x2178)
	MemberAtOffset(int, SpectatedCount, 0x18ec)
	MemberAtOffset(FRotator, AimOffsets, 0x1580)
	,
	bool GetTslWeapon_Trajectory(ATslWeapon_Trajectory& OutTslWeapon) const;
	bool GetTslWeapon_Trajectory_Next(ATslWeapon_Trajectory& OutTslWeapon, unsigned Next) const;
	bool GetTslWeapon(ATslWeapon& OutTslWeapon) const;
)

//TslGame.TslWheeledVehicle.VehicleCommonComponent 
DefClass(ATslWheeledVehicle, APawn,
	MemberAtOffset(NativePtr<UTslVehicleCommonComponent>, VehicleCommonComponent, 0xae8)
)

//TslGame.TslFloatingVehicle.VehicleCommonComponent 
DefClass(ATslFloatingVehicle, APawn,
	MemberAtOffset(NativePtr<UTslVehicleCommonComponent>, VehicleCommonComponent, 0x4d0)
)

//Function Engine.ActorComponent.GetOwner 
DefClass(UActorComponent, UObject,
	MemberAtOffset(NativePtr<AActor>, Owner, 0x1C8)
)

//TslGame.TslVehicleCommonComponent.Health 
//TslGame.TslVehicleCommonComponent.HealthMax 
//TslGame.TslVehicleCommonComponent.Fuel 
//TslGame.TslVehicleCommonComponent.FuelMax 
DefClass(UTslVehicleCommonComponent, UActorComponent,
	MemberAtOffset(float, Health, 0x2e0)
	MemberAtOffset(float, HealthMax, 0x2e4)
	MemberAtOffset(float, Fuel, 0x2e8)
	MemberAtOffset(float, FuelMax, 0x2ec)
)

//Engine.SceneComponent.ComponentVelocity 
//Engine.SceneComponent.AttachParent 
//Function Engine.SceneComponent.K2_GetComponentToWorld 
DefClass(USceneComponent, UActorComponent,
	MemberAtOffset(FVector, ComponentVelocity, 0x254)
	MemberAtOffset(NativePtr<USceneComponent>, AttachParent, 0x290)

	MemberAtOffset(FTransform, ComponentToWorld, 0x330)
	,
	FTransform GetSocketTransform(FName SocketName, ERelativeTransformSpace TransformSpace) const;
)

//Engine.PrimitiveComponent.LastSubmitTime 
//Engine.PrimitiveComponent.LastRenderTimeOnScreen 
DefClass(UPrimitiveComponent, USceneComponent,
	MemberAtOffset(float, LastSubmitTime, 0x788)
	MemberAtOffset(float, LastRenderTimeOnScreen, 0x790)
	,
	bool IsVisible() const { return LastRenderTimeOnScreen + 0.05f >= LastSubmitTime; }
)

DefClass(UShapeComponent, UPrimitiveComponent, )
DefClass(UCapsuleComponent, UShapeComponent, )
DefClass(UMeshComponent, UPrimitiveComponent, )

//Engine.SkinnedMeshComponent.SkeletalMesh 
DefClass(USkinnedMeshComponent, UMeshComponent,
	MemberAtOffset(NativePtr<USkeletalMesh>, SkeletalMesh, 0xae8)

	MemberAtOffset(TArray<FTransform>, BoneSpaceTransforms, 0xAF8)
	,
	FTransform GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace) const;
	FTransform GetBoneTransform(int32 BoneIdx, const FTransform& LocalToWorld) const;
	FTransform GetBoneTransform(int32 BoneIdx) const;
	bool GetSocketInfoByName(FName InSocketName, FTransform& OutTransform, int32& OutBoneIndex, USkeletalMeshSocket& OutSocket) const;
	int32 GetBoneIndex(FName BoneName) const;
	FName GetParentBone(FName BoneName) const;
)

//Engine.SkeletalMeshComponent.AnimScriptInstance 
DefClass(USkeletalMeshComponent, USkinnedMeshComponent,
	MemberAtOffset(NativePtr<UAnimInstance>, AnimScriptInstance, 0xca0)
)

//Engine.StaticMeshComponent.StaticMesh 
DefClass(UStaticMeshComponent, UMeshComponent,
	MemberAtOffset(NativePtr<UStaticMesh>, StaticMesh, 0xaf8)
	,
	FTransform GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace) const;
	bool GetSocketByName(FName InSocketName, UStaticMeshSocket& OutSocket) const;
)

//Engine.SkeletalMeshSocket.SocketName 
//Engine.SkeletalMeshSocket.BoneName 
//Engine.SkeletalMeshSocket.RelativeLocation 
//Engine.SkeletalMeshSocket.RelativeRotation 
//Engine.SkeletalMeshSocket.RelativeScale 
DefClass(USkeletalMeshSocket, UObject,
	MemberAtOffsetZero(FName, SocketName, 0x40)
	MemberAtOffset(FName, BoneName, 0x48)
	MemberAtOffset(FVector, RelativeLocation, 0x50)
	MemberAtOffset(FRotator, RelativeRotation, 0x5c)
	MemberAtOffset(FVector, RelativeScale, 0x68)
	,
	FTransform GetSocketLocalTransform() const;
)

//Engine.StaticMeshSocket.SocketName 
//Engine.StaticMeshSocket.RelativeLocation 
//Engine.StaticMeshSocket.RelativeRotation 
//Engine.StaticMeshSocket.RelativeScale 
DefClass(UStaticMeshSocket, UObject,
	MemberAtOffsetZero(FName, SocketName, 0x40)
	MemberAtOffset(FVector, RelativeLocation, 0x48)
	MemberAtOffset(FRotator, RelativeRotation, 0x54)
	MemberAtOffset(FVector, RelativeScale, 0x60)
	,
	bool GetSocketTransform(FTransform& OutTransform, const UStaticMeshComponent& MeshComp) const;
)

//Engine.SkeletalMesh.Skeleton 
//Engine.SkeletalMesh.Sockets 
//Function Engine.SkinnedMeshComponent.GetNumBones -> Get FinalRefBoneInfo Offset(-8)
DefClass(USkeletalMesh, UObject,
	MemberAtOffset(NativePtr<USkeleton>, Skeleton, 0x60)
	MemberAtOffset(TArray<NativePtr<USkeletalMeshSocket>>, Sockets, 0x2d8)

	MemberAtOffset(TArray<FMeshBoneInfo>, FinalRefBoneInfo, 0x160 - 0x8)
	,
	bool FindSocketInfo(FName InSocketName, FTransform& OutTransform, int32& OutBoneIndex, int32& OutIndex, USkeletalMeshSocket& OutSocket) const;
	int FindBoneIndex(FName BoneName) const;
	FName GetBoneName(const int32 BoneIndex) const;
	int32 GetParentIndex(const int32 BoneIndex) const;
	int32 GetParentIndexInternal(const int32 BoneIndex, const TArray<FMeshBoneInfo>& BoneInfo) const;
)

//Engine.StaticMesh.Sockets 
DefClass(UStaticMesh, UObject,
	MemberAtOffset(TArray<NativePtr<UStaticMeshSocket>>, Sockets, 0xd8)
	,
	bool FindSocket(FName InSocketName, UStaticMeshSocket& OutSocket) const;
)

//TslGame.WeaponMeshComponent.AttachedStaticComponentMap 
DefClass(UWeaponMeshComponent, USkeletalMeshComponent,
	MemberAtOffset(UNPACK(TMap<TEnumAsByte<EWeaponAttachmentSlotID>, NativePtr<UStaticMeshComponent>>), AttachedStaticComponentMap, 0x11e8)
	,
	NativePtr<UStaticMeshComponent> GetStaticMeshComponentScopeType() const;
float GetScopingAttachPointRelativeZ(FName ScopingAttachPoint) const;
)

//Engine.Skeleton.Sockets 
DefClass(USkeleton, UObject,
	MemberAtOffset(TArray<NativePtr<USkeletalMeshSocket>>, Sockets, 0x1a8)
	,
	bool FindSocketAndIndex(FName InSocketName, int32& OutIndex, USkeletalMeshSocket& Socket) const;
)

//TslGame.DroppedItem.Item 
DefClass(ADroppedItem, AActor,
	MemberAtOffset(EncryptedPtr<UItem>, Item, 0x438)
)

//TslGame.DroppedItemInteractionComponent.Item 
DefClass(UDroppedItemInteractionComponent, USceneComponent,
	MemberAtOffset(NativePtr<UItem>, Item, 0x748)
)

//Function TslGame.Item.BP_GetItemID 
DefBaseClass(FItemTableRowBase,
	MemberAtOffset(FName, ItemID, 0x248)
)

//Function TslGame.Item.BP_GetItemID 
DefClass(UItem, UObject,
	MemberAtOffset(NativePtr<FItemTableRowBase>, ItemTable, 0xC0)
	,
	FName GetItemID() const;
	unsigned GetHash() const;
	ItemInfo GetInfo() const;
)

//TslGame.VehicleRiderComponent.SeatIndex 
//TslGame.VehicleRiderComponent.LastVehiclePawn 
DefClass(UVehicleRiderComponent, UActorComponent,
	MemberAtOffset(int, SeatIndex, 0x238)
	MemberAtOffset(NativePtr<APawn>, LastVehiclePawn, 0x270)
)

//TslGame.WeaponProcessorComponent.EquippedWeapons 
//Function TslGame.WeaponProcessorComponent.GetWeaponIndex 
DefClass(UWeaponProcessorComponent, UActorComponent,
	MemberAtOffset(TArray<NativePtr<ATslWeapon>>, EquippedWeapons, 0x2c8)
	MemberAtOffset(uint8_t, WeaponArmInfo_RightWeaponIndex, 0x2e8 + 0x1)
)

//Function TslGame.AttachableItem.GetAttachmentData 
DefClass(UAttachableItem, UItem,
	MemberAtOffset(NativePtr<FItemTableRowAttachment>, WeaponAttachmentData, 0x138)
)

DefClass(FItemTableRowAttachment, FItemTableRowBase,)

//TslGame.TslWeapon.AttachedItems 
//TslGame.TslWeapon.Mesh3P 
//TslGame.TslWeapon.WeaponTag 
//TslGame.TslWeapon.FiringAttachPoint 
DefClass(ATslWeapon, AActor,
	MemberAtOffset(TArray<NativePtr<UAttachableItem>>, AttachedItems, 0x7e8)
	MemberAtOffset(EncryptedPtr<UWeaponMeshComponent>, Mesh3P, 0x7a0)
	MemberAtOffset(FName, WeaponTag, 0x7f8)
	MemberAtOffset(FName, FiringAttachPoint, 0x840)
	,
	tWeaponInfo GetWeaponInfo() const;
)

//TslGame.TslWeapon_Gun.CurrentZeroLevel 
//TslGame.TslWeapon_Gun.ScopingAttachPoint 
//TslGame.TslWeapon_Gun.CurrentAmmoData
//TslGame.TslWeapon_Gun.CurrentState
//Function TslGame.TslWeapon_Gun.GetCurrentScopeZeroingLevel 
//Function TslGame.TslWeapon_Gun.WeaponCycleDone 
DefClass(ATslWeapon_Gun, ATslWeapon,
	MemberAtOffset(int, CurrentZeroLevel, 0xa0c)
	MemberAtOffset(FName, ScopingAttachPoint, 0xbf8)
	MemberAtOffset(uint16_t, CurrentAmmoData, 0xa08)
	MemberAtOffset(TEnumAsByte<EWeaponState>, CurrentState, 0xa02)
	 
	MemberAtOffset(TEnumAsByte<bool>, bCantedSighted, 0xE85)
	MemberAtOffset(TEnumAsByte<bool>, bWeaponCycleDone, 0xA2D)
	,
	int GetCurrentAmmo() const { return HIBYTE(CurrentAmmoData) ? -1 : CurrentAmmoData; }
	ESight GetSight() const;
	float GetZeroingDistance(bool IsScoping) const;
	bool IsProperForAutoClick(bool IsScoping) const;
)

//TslGame.TslWeapon_Trajectory.WeaponTrajectoryData 
//TslGame.TslWeapon_Trajectory.TrajectoryGravityZ 
DefClass(ATslWeapon_Trajectory, ATslWeapon_Gun,
	MemberAtOffset(NativePtr<UWeaponTrajectoryData>, WeaponTrajectoryData, 0xff8)
	MemberAtOffset(float, TrajectoryGravityZ, 0xf4c)
)

DefClass(ATslWeapon_Throwable, ATslWeapon_Gun, );
DefClass(ATslWeapon_Melee, ATslWeapon, );

DefClass(UAnimInstance, UObject, )

//TslGame.TslAnimInstance.ControlRotation_CP 
//TslGame.TslAnimInstance.RecoilADSRotation_CP 
//TslGame.TslAnimInstance.LeanLeftAlpha_CP 
//TslGame.TslAnimInstance.LeanRightAlpha_CP 
//TslGame.TslAnimInstance.bIsScoping_CP 
DefClass(UTslAnimInstance, UAnimInstance,
	MemberAtOffset(FRotator, ControlRotation_CP, 0x744)
	MemberAtOffset(FRotator, RecoilADSRotation_CP, 0x9d4)
	MemberAtOffset(float, LeanLeftAlpha_CP, 0xde4)
	MemberAtOffset(float, LeanRightAlpha_CP, 0xde8)
	MemberAtOffset(TEnumAsByte<bool>, bIsScoping_CP, 0xcfe)
	MemberAtOffset(TEnumAsByte<bool>, bLocalFPP_CP, 0xf28)
)

//TslGame.WeaponTrajectoryData.TrajectoryConfig 
DefClass(UWeaponTrajectoryData, UObject,
	MemberAtOffset(FWeaponTrajectoryConfig, TrajectoryConfig, 0x118)
)

//TslGame.ItemPackage.Items 
DefClass(AItemPackage, AActor,
	MemberAtOffset(TArray<NativePtr<UItem>>, Items, 0x578)
)

//TslGame.TslPlayerState.PlayerStatistics 
//TslGame.TslPlayerState.DamageDealtOnEnemy 
DefClass(ATslPlayerState, APlayerState,
	MemberAtOffset(float, DamageDealtOnEnemy, 0xa5c)
	MemberAtOffset(FTslPlayerStatistics, PlayerStatistics, 0x4a8)
	MemberAtOffset(FWuPubgIdData, PubgIdData, 0xc90)
)

constexpr auto offset1 = offsetof(ATslPlayerState, DamageDealtOnEnemy);
constexpr auto offset2 = offsetof(ATslPlayerState, PlayerStatistics);
constexpr auto offset3 = offsetof(ATslPlayerState, PubgIdData);

//Engine.CurveVector.FloatCurves 
DefClass(UCurveVector, UObject,
	MemberAtOffset(FRichCurve, FloatCurves, 0x48)
)

DefClass(AHUD, AActor, )
DefClass(ATslBaseHUD, AHUD,
	MemberAtOffset(UNPACK(TMap<FString, FTslWidgetState>), WidgetStateMap, 0x508)
	,
	void EnumAllWidget(tl::function<bool(std::wstring Key, NativePtr<UUserWidget> WidgetPtr)> CallBack) const;
)

DefClass(UClass, UObject, )
DefClass(UVisual, UObject, )
DefClass(UWidget, UVisual, )
DefClass(UUserWidget, UWidget, )
DefClass(UTslUserWidget, UUserWidget, )
DefClass(UUmgBaseWidget, UTslUserWidget, )
DefClass(UMinimapCanvasWidget, UUserWidget,
	MemberAtOffset(int, SelectMinimapSizeIndex, 0x5d0)
)

DefClass(UMinimapOriginalType_C, UMinimapCanvasWidget, )
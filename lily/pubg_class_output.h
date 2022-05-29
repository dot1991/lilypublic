class ATslLivingThing
//function ATslLivingThing.GetWorldTimeSeconds 0x4eb4e80

class UWorld
MemberAtOffset(EncryptedPtr<ULevel>, CurrentLevel, 0x140)

class AController
MemberAtOffset(EncryptedPtr<ACharacter>, Character, 0x458)
MemberAtOffset(EncryptedPtr<APawn>, Pawn, 0x448)

class UPlayerInput
//function UPlayerInput.SetMouseSensitivity 0x5c16154

class APlayerController
MemberAtOffset(NativePtr<APawn>, SpectatorPawn, 0x748)
MemberAtOffset(NativePtr<APlayerCameraManager>, PlayerCameraManager, 0x4b8)
MemberAtOffset(NativePtr<UPlayerInput>, PlayerInput, 0x530)
MemberAtOffset(NativePtr<AHUD>, MyHUD, 0x4b0)

class ATslPlayerController
MemberAtOffset(float, DefaultFOV, 0xacc)

class APlayerCameraManager
MemberAtOffset(float, CameraCache_POV_FOV, 0x1650 + 0x10 + 0x598)
MemberAtOffset(FRotator, CameraCache_POV_Rotation, 0x1650 + 0x10 + 0x5a8)
MemberAtOffset(FVector, CameraCache_POV_Location, 0x1650 + 0x10 + 0x588)

class AActor
MemberAtOffset(EncryptedPtr<USceneComponent>, RootComponent, 0xf8)
MemberAtOffset(FRepMovement, ReplicatedMovement, 0x78)

class APawn
MemberAtOffset(EncryptedPtr<APlayerState>, PlayerState, 0x430)

class ACharacter
MemberAtOffset(NativePtr<USkeletalMeshComponent>, Mesh, 0x4c0)
MemberAtOffset(FVector, BaseTranslationOffset, 0x4cc)

class ATslCharacter
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

class ATslWheeledVehicle
MemberAtOffset(NativePtr<UTslVehicleCommonComponent>, VehicleCommonComponent, 0xae8)

class ATslFloatingVehicle
MemberAtOffset(NativePtr<UTslVehicleCommonComponent>, VehicleCommonComponent, 0x4d0)

class UActorComponent
//function UActorComponent.GetOwner 0xc2e9dc

class UTslVehicleCommonComponent
MemberAtOffset(float, Health, 0x2e0)
MemberAtOffset(float, HealthMax, 0x2e4)
MemberAtOffset(float, Fuel, 0x2e8)
MemberAtOffset(float, FuelMax, 0x2ec)

class USceneComponent
//function USceneComponent.K2_GetComponentToWorld 0x5c09e98
MemberAtOffset(FVector, ComponentVelocity, 0x254)
MemberAtOffset(NativePtr<USceneComponent>, AttachParent, 0x290)

class UPrimitiveComponent
MemberAtOffset(float, LastSubmitTime, 0x788)
MemberAtOffset(float, LastRenderTimeOnScreen, 0x790)

class USkinnedMeshComponent
MemberAtOffset(NativePtr<USkeletalMesh>, SkeletalMesh, 0xae8)

class USkeletalMeshComponent
MemberAtOffset(NativePtr<UAnimInstance>, AnimScriptInstance, 0xca0)

class UStaticMeshComponent
MemberAtOffset(NativePtr<UStaticMesh>, StaticMesh, 0xaf8)

class USkeletalMeshSocket
MemberAtOffset(FName, SocketName, 0x40)
MemberAtOffset(FName, BoneName, 0x48)
MemberAtOffset(FVector, RelativeLocation, 0x50)
MemberAtOffset(FRotator, RelativeRotation, 0x5c)
MemberAtOffset(FVector, RelativeScale, 0x68)

class UStaticMeshSocket
MemberAtOffset(FName, SocketName, 0x40)
MemberAtOffset(FVector, RelativeLocation, 0x48)
MemberAtOffset(FRotator, RelativeRotation, 0x54)
MemberAtOffset(FVector, RelativeScale, 0x60)

class USkeletalMesh
MemberAtOffset(NativePtr<USkeleton>, Skeleton, 0x60)
MemberAtOffset(TArray<NativePtr<USkeletalMeshSocket>>, Sockets, 0x2d8)

class USkinnedMeshComponent
//function USkinnedMeshComponent.GetNumBones 0x5c3b368

class UStaticMesh
MemberAtOffset(TArray<NativePtr<UStaticMeshSocket>>, Sockets, 0xd8)

class UWeaponMeshComponent
MemberAtOffset(UNPACK(TMap<TEnumAsByte<EWeaponAttachmentSlotID>,NativePtr<UStaticMeshComponent>>), AttachedStaticComponentMap, 0x11e8)

class USkeleton
MemberAtOffset(TArray<NativePtr<USkeletalMeshSocket>>, Sockets, 0x1a8)

class ADroppedItem
MemberAtOffset(EncryptedPtr<UItem>, Item, 0x438)

class UDroppedItemInteractionComponent
MemberAtOffset(NativePtr<UItem>, Item, 0x748)

struct FItemTableRowBase
MemberAtOffset(FName, ItemID, 0x248)

class UItem
//function UItem.BP_GetItemID 0x4e14ea0

class UVehicleRiderComponent
MemberAtOffset(int, SeatIndex, 0x238)
MemberAtOffset(NativePtr<APawn>, LastVehiclePawn, 0x270)

class UWeaponProcessorComponent
MemberAtOffset(TArray<NativePtr<ATslWeapon>>, EquippedWeapons, 0x2c8)
MemberAtOffset(uint8_t, WeaponArmInfo_RightWeaponIndex, 0x2e8 + 0x1)

class UAttachableItem
//function UAttachableItem.GetAttachmentData 0x4e6e104

class ATslWeapon
MemberAtOffset(TArray<NativePtr<UAttachableItem>>, AttachedItems, 0x7e8)
MemberAtOffset(EncryptedPtr<UWeaponMeshComponent>, Mesh3P, 0x7a0)
MemberAtOffset(FName, WeaponTag, 0x7f8)
MemberAtOffset(FName, FiringAttachPoint, 0x840)

class ATslWeapon_Gun
MemberAtOffset(int, CurrentZeroLevel, 0xa0c)
MemberAtOffset(FName, ScopingAttachPoint, 0xbf8)
MemberAtOffset(uint16_t, CurrentAmmoData, 0xa08)
MemberAtOffset(TEnumAsByte<EWeaponState>, CurrentState, 0xa02)
//function ATslWeapon_Gun.GetCurrentScopeZeroingLevel 0x4e6f5f8
//function ATslWeapon_Gun.WeaponCycleDone 0xea4374

class ATslWeapon_Trajectory
MemberAtOffset(NativePtr<UWeaponTrajectoryData>, WeaponTrajectoryData, 0xff8)
MemberAtOffset(float, TrajectoryGravityZ, 0xf4c)

class UTslAnimInstance
MemberAtOffset(FRotator, ControlRotation_CP, 0x744)
MemberAtOffset(FRotator, RecoilADSRotation_CP, 0x9d4)
MemberAtOffset(float, LeanLeftAlpha_CP, 0xde4)
MemberAtOffset(float, LeanRightAlpha_CP, 0xde8)
MemberAtOffset(TEnumAsByte<bool>, bIsScoping_CP, 0xcfe)
MemberAtOffset(TEnumAsByte<bool>, bLocalFPP_CP, 0xf28)

class UWeaponTrajectoryData
MemberAtOffset(FWeaponTrajectoryConfig, TrajectoryConfig, 0x118)

class AItemPackage
MemberAtOffset(TArray<NativePtr<UItem>>, Items, 0x578)

class ATslPlayerState
MemberAtOffset(float, DamageDealtOnEnemy, 0xa5c)
MemberAtOffset(FTslPlayerStatistics, PlayerStatistics, 0x4a8)
MemberAtOffset(FWuPubgIdData, PubgIdData, 0xc90)

class UCurveVector
MemberAtOffset(FRichCurve, FloatCurves, 0x48)

class ATslBaseHUD
MemberAtOffset(UNPACK(TMap<FString,FTslWidgetState>), WidgetStateMap, 0x508)

class UMinimapCanvasWidget
MemberAtOffset(int, SelectMinimapSizeIndex, 0x5d0)

class ATslLivingThing
function GetWorldTimeSeconds

class UWorld
EncryptedPtr<ULevel> CurrentLevel

class AController
EncryptedPtr<ACharacter> Character
EncryptedPtr<APawn> Pawn

class UPlayerInput
function SetMouseSensitivity

class APlayerController
NativePtr<APawn> SpectatorPawn
NativePtr<APlayerCameraManager> PlayerCameraManager
NativePtr<UPlayerInput> PlayerInput
NativePtr<AHUD> MyHUD

class ATslPlayerController
float DefaultFOV

class APlayerCameraManager
float CameraCache + CameraCacheEntry.POV + MinimalViewInfo.FOV
FRotator CameraCache + CameraCacheEntry.POV + MinimalViewInfo.Rotation
FVector CameraCache + CameraCacheEntry.POV + MinimalViewInfo.Location

class AActor
EncryptedPtr<USceneComponent> RootComponent
FRepMovement ReplicatedMovement

class APawn
EncryptedPtr<APlayerState> PlayerState

class ACharacter
NativePtr<USkeletalMeshComponent> Mesh
FVector BaseTranslationOffset

class ATslCharacter
float Health
float HealthMax
float GroggyHealth
float GroggyHealthMax
FString CharacterName
int LastTeamNum
NativePtr<UVehicleRiderComponent> VehicleRiderComponent
NativePtr<UWeaponProcessorComponent> WeaponProcessor
int SpectatedCount
FRotator AimOffsets

class ATslWheeledVehicle
NativePtr<UTslVehicleCommonComponent> VehicleCommonComponent

class ATslFloatingVehicle
NativePtr<UTslVehicleCommonComponent> VehicleCommonComponent

class UActorComponent
function GetOwner

class UTslVehicleCommonComponent
float Health
float HealthMax
float Fuel
float FuelMax

class USceneComponent
function K2_GetComponentToWorld
FVector ComponentVelocity
NativePtr<USceneComponent> AttachParent

class UPrimitiveComponent
float LastSubmitTime
float LastRenderTimeOnScreen

class USkinnedMeshComponent
NativePtr<USkeletalMesh> SkeletalMesh

class USkeletalMeshComponent
NativePtr<UAnimInstance> AnimScriptInstance

class UStaticMeshComponent
NativePtr<UStaticMesh> StaticMesh

class USkeletalMeshSocket
FName SocketName
FName BoneName
FVector RelativeLocation
FRotator RelativeRotation
FVector RelativeScale

class UStaticMeshSocket
FName SocketName
FVector RelativeLocation
FRotator RelativeRotation
FVector RelativeScale

class USkeletalMesh
NativePtr<USkeleton> Skeleton
TArray<NativePtr<USkeletalMeshSocket>> Sockets

class USkinnedMeshComponent
function GetNumBones

class UStaticMesh
TArray<NativePtr<UStaticMeshSocket>> Sockets

class UWeaponMeshComponent
UNPACK(TMap<TEnumAsByte<EWeaponAttachmentSlotID>,NativePtr<UStaticMeshComponent>>) AttachedStaticComponentMap

class USkeleton
TArray<NativePtr<USkeletalMeshSocket>> Sockets

class ADroppedItem
EncryptedPtr<UItem> Item

class UDroppedItemInteractionComponent
NativePtr<UItem> Item

struct ItemTableRowBase
FName ItemID

class UItem
function BP_GetItemID

class UVehicleRiderComponent
int SeatIndex
NativePtr<APawn> LastVehiclePawn

class UWeaponProcessorComponent
TArray<NativePtr<ATslWeapon>> EquippedWeapons
uint8_t WeaponArmInfo + WeaponArmInfo.RightWeaponIndex

class UAttachableItem
function GetAttachmentData

class ATslWeapon
TArray<NativePtr<UAttachableItem>> AttachedItems
EncryptedPtr<UWeaponMeshComponent> Mesh3P
FName WeaponTag
FName FiringAttachPoint

class ATslWeapon_Gun
int CurrentZeroLevel
FName ScopingAttachPoint
uint16_t CurrentAmmoData
TEnumAsByte<EWeaponState> CurrentState
function GetCurrentScopeZeroingLevel
function WeaponCycleDone

class ATslWeapon_Trajectory
NativePtr<UWeaponTrajectoryData> WeaponTrajectoryData
float TrajectoryGravityZ

class UTslAnimInstance
FRotator ControlRotation_CP
FRotator RecoilADSRotation_CP
float LeanLeftAlpha_CP
float LeanRightAlpha_CP
TEnumAsByte<bool> bIsScoping_CP
TEnumAsByte<bool> bLocalFPP_CP

class UWeaponTrajectoryData
FWeaponTrajectoryConfig TrajectoryConfig

class AItemPackage
TArray<NativePtr<UItem>> Items

class ATslPlayerState
float DamageDealtOnEnemy
FTslPlayerStatistics PlayerStatistics
FWuPubgIdData PubgIdData

class UCurveVector
FRichCurve FloatCurves

class ATslBaseHUD
UNPACK(TMap<FString,FTslWidgetState>) WidgetStateMap

class UMinimapCanvasWidget
int SelectMinimapSizeIndex
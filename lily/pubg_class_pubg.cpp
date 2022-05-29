#include "pubg_class.h"

void SimulateWeaponTrajectory(FVector Direction, float Distance, float TrajectoryGravityZ, float BallisticDragScale, float BallisticDropScale, 
	float BDS, float SimulationSubstepTime, float VDragCoefficient, const FRichCurve& FloatCurve, float& TravelTime, float& BulletDrop) 
{
	TravelTime = 0.0f;
	BulletDrop = 0.0f;

	float TravelDistance = 0.0f;
	float CurrentDrop = 0.0f;

	Direction.Normalize();
	Direction = Direction * 100.0f;

	for (auto i = 0; i < 500; i++) {
		float BulletSpeed = FloatCurve.Eval(TravelDistance * BDS * BallisticDragScale, 0.0f);
		FVector Velocity = Direction * BulletSpeed;
		Velocity.Z += CurrentDrop;

		FVector Acceleration = Velocity * SimulationSubstepTime;
		float AccelerationLen = Acceleration.Length() / 100.0f;
		if (TravelDistance + AccelerationLen > Distance)
			break;

		TravelDistance += AccelerationLen;
		TravelTime += SimulationSubstepTime;
		BulletDrop += SimulationSubstepTime * CurrentDrop;
		CurrentDrop += SimulationSubstepTime * TrajectoryGravityZ * 100.0f * VDragCoefficient * BallisticDropScale;
	}
}

std::pair<float, float> GetBulletDropAndTravelTime(const FVector& GunLocation, const FRotator& GunRotation, const FVector& TargetPos,
	float ZeroingDistance, float BulletDropAdd, float InitialSpeed, float TrajectoryGravityZ, float BallisticDragScale,
	float BallisticDropScale, float BDS, float SimulationSubstepTime, float VDragCoefficient, NativePtr<UCurveVector> BallisticCurve)
{
	const float ZDistanceToTarget = TargetPos.Z - GunLocation.Z;
	const float DistanceToTarget = GunLocation.Distance(TargetPos) / 100.0f;
	
	float TravelTime = DistanceToTarget / InitialSpeed;
	float BulletDrop = 0.5f * TrajectoryGravityZ * TravelTime * TravelTime * 100.0f;

	float TravelTimeZero = ZeroingDistance / InitialSpeed;
	float BulletDropZero = 0.5f * TrajectoryGravityZ * TravelTimeZero * TravelTimeZero * 100.0f;

	UCurveVector CurveVector;
	if (BallisticCurve.Read(CurveVector)) {
		SimulateWeaponTrajectory(GunRotation.GetUnitVector(), DistanceToTarget, TrajectoryGravityZ, BallisticDragScale, BallisticDropScale, BDS, SimulationSubstepTime, VDragCoefficient, CurveVector.FloatCurves, TravelTime, BulletDrop);
		SimulateWeaponTrajectory(FVector(1.0f, 0.0f, 0.0f), ZeroingDistance, TrajectoryGravityZ, BallisticDragScale, BallisticDropScale, BDS, SimulationSubstepTime, VDragCoefficient, CurveVector.FloatCurves, TravelTimeZero, BulletDropZero);
	}

	BulletDrop = fabsf(BulletDrop) - fabsf(BulletDropAdd);
	if (BulletDrop < 0.0f)
		BulletDrop = 0.0f;
	BulletDropZero = fabsf(BulletDropZero) + fabsf(BulletDropAdd);

	const float TargetPitch = asinf((ZDistanceToTarget + BulletDrop) / 100.0f / DistanceToTarget);
	const float ZeroPitch = IsNearlyZero(ZeroingDistance) ? 0.0f : atan2f(BulletDropZero / 100.0f, ZeroingDistance);
	const float FinalPitch = TargetPitch - ZeroPitch;
	const float AdditiveZ = DistanceToTarget * sinf(FinalPitch) * 100.0f - ZDistanceToTarget;

	return std::pair(AdditiveZ , TravelTime);
}

FName UItem::GetItemID() const {
	FItemTableRowBase ItemTableRowBase;
	if (!ItemTable.Read(ItemTableRowBase))
		return { 0, 0 };
	return ItemTableRowBase.ItemID;
}

unsigned UItem::GetHash() const {
	return GetItemID().GetHash();
}

ItemInfo UItem::GetInfo() const {
	return GetItemInfo(GetHash());
}

tWeaponInfo ATslWeapon::GetWeaponInfo() const {
	return ::GetWeaponInfo(GetFName().GetHash());
}

ESight ATslWeapon_Gun::GetSight() const {
	for (const auto& AttachableItemPtr : AttachedItems.GetVector()) {
		UAttachableItem AttachableItem;
		if (!AttachableItemPtr.Read(AttachableItem))
			continue;

		FItemTableRowAttachment ItemTableRowAttachment;
		if (!AttachableItem.WeaponAttachmentData.Read(ItemTableRowAttachment))
			continue;

		switch (ItemTableRowAttachment.ItemID.GetHash()) {
			HASH_CASE("Item_Attach_Weapon_Upper_PM2_01_C"h, ESight::X15);
			HASH_CASE("Item_Attach_Weapon_Upper_CQBSS_C"h, ESight::X8);
			HASH_CASE("Item_Attach_Weapon_Upper_Scope6x_C"h, ESight::X6);
			HASH_CASE("Item_Attach_Weapon_Upper_ACOG_01_C"h, ESight::X4);
			HASH_CASE("Item_Attach_Weapon_Upper_Scope3x_C"h, ESight::X3);
			HASH_CASE("Item_Attach_Weapon_Upper_Aimpoint_C"h, ESight::X2);
			HASH_CASE("Item_Attach_Weapon_Upper_DotSight_01_C"h, ESight::RedDot);
			HASH_CASE("Item_Attach_Weapon_Upper_Holosight_C"h, ESight::Holo);
		}
	}

	return ESight::Iron;
}

float ATslWeapon_Gun::GetZeroingDistance(bool IsScoping) const {
	if (bCantedSighted)
		return GetFName().GetHash() == "WeapP90_C"h ? 100.0f : 50.0f;

	auto ZeroingInfo = GetWeaponInfo().ZeroingInfo;
	int ZeroLevel = IsScoping ? CurrentZeroLevel : 0;

	switch (GetSight()) {
	case ESight::X15:
	case ESight::X8:
		return ZeroingInfo.BaseScope + ZeroingInfo.Increment * ZeroLevel;
	case ESight::X6:
	case ESight::X4:
	case ESight::X3:
	case ESight::X2:
	case ESight::RedDot:
		return ZeroingInfo.BaseScope;
	case ESight::Holo:
		return ZeroingInfo.BaseHolo;
	}

	return ZeroingInfo.BaseIron + ZeroingInfo.Increment * ZeroLevel;
}

bool ATslWeapon_Gun::IsProperForAutoClick(bool IsScoping) const {
	if (!IsScoping)
		return true;

	if (bCantedSighted)
		return true;

	switch (GetSight()) {
	case ESight::X15:
	case ESight::X8:
	case ESight::X6:
	case ESight::X4:
	case ESight::X3:
		return false;
	}

	return true;
}

NativePtr<UStaticMeshComponent> UWeaponMeshComponent::GetStaticMeshComponentScopeType() const {
	NativePtr<UStaticMeshComponent> Result = 0;
	AttachedStaticComponentMap.GetValue(EWeaponAttachmentSlotID::UpperRail, Result);
	return Result;
}

float UWeaponMeshComponent::GetScopingAttachPointRelativeZ(FName ScopingAttachPoint) const {
	float Default = 15.0f;

	UStaticMeshComponent StaticMeshComponent;
	if (GetStaticMeshComponentScopeType().Read(StaticMeshComponent)) {
		//Scope Attached
		const float RelativeZ_1 = StaticMeshComponent.GetSocketTransform(ScopingAttachPoint, RTS_Component).Translation.Z;
		const float RelativeZ_2 = StaticMeshComponent.ComponentToWorld.GetRelativeTransform(ComponentToWorld).Translation.Z;
		return RelativeZ_1 + RelativeZ_2;
	}
	else
		return GetSocketTransform(ScopingAttachPoint, RTS_Component).Translation.Z;

	return Default;
}

bool ATslCharacter::GetTslWeapon_Trajectory(ATslWeapon_Trajectory& OutTslWeapon) const {
	UWeaponProcessorComponent WeaponProcessorComponent;
	if (!WeaponProcessor.Read(WeaponProcessorComponent))
		return false;

	BYTE WeaponIndex = WeaponProcessorComponent.WeaponArmInfo_RightWeaponIndex;
	if (WeaponIndex != 0 && WeaponIndex != 1 && WeaponIndex != 2)
		return false;

	NativePtr<ATslWeapon> TslWeaponPtr;
	if (!WeaponProcessorComponent.EquippedWeapons.GetValue(WeaponIndex, TslWeaponPtr))
		return false;

	return TslWeaponPtr.ReadOtherType(OutTslWeapon);
}

bool ATslCharacter::GetTslWeapon_Trajectory_Next(ATslWeapon_Trajectory& OutTslWeapon, unsigned N) const {
	UWeaponProcessorComponent WeaponProcessorComponent;
	if (!WeaponProcessor.Read(WeaponProcessorComponent))
		return false;

	BYTE WeaponIndex = WeaponProcessorComponent.WeaponArmInfo_RightWeaponIndex;

	unsigned Count = 0;
	for (unsigned i = 0; i < 3; i++) {
		if (i == WeaponIndex)
			continue;

		NativePtr<ATslWeapon> TslWeaponPtr;
		if (!WeaponProcessorComponent.EquippedWeapons.GetValue(i, TslWeaponPtr))
			continue;

		if (!TslWeaponPtr.ReadOtherType(OutTslWeapon))
			continue;

		Count++;
		if (Count > N)
			return true;
	}

	return false;
}

bool ATslCharacter::GetTslWeapon(ATslWeapon& OutTslWeapon) const {
	UWeaponProcessorComponent WeaponProcessorComponent;
	if (!WeaponProcessor.Read(WeaponProcessorComponent))
		return false;

	NativePtr<ATslWeapon> TslWeaponPtr;
	if (!WeaponProcessorComponent.EquippedWeapons.GetValue(WeaponProcessorComponent.WeaponArmInfo_RightWeaponIndex, TslWeaponPtr))
		return false;

	return TslWeaponPtr.ReadOtherType(OutTslWeapon);
}

void ATslBaseHUD::EnumAllWidget(tl::function<bool(std::wstring Key, NativePtr<UUserWidget> WidgetPtr)> CallBack) const {
	for (auto& Elem : WidgetStateMap.GetVector()) {
		auto& Key = Elem.Value.Key;
		auto& Value = Elem.Value.Value;
		if (!CallBack(Key.GetString(), Value.Widget))
			break;
	}
}
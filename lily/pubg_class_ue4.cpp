#include "pubg_class.h"

constexpr inline auto NAME_None = FName(0, 0);
constexpr inline auto INDEX_NONE = -1;

FVector WorldToScreen(const FVector& WorldLocation, const FMatrix& RotationMatrix, const FVector& CameraLocation, float CameraFOV, float Width, float Height) {
	FVector Screenlocation;

	FVector AxisX = RotationMatrix.GetScaledAxisX();
	FVector AxisY = RotationMatrix.GetScaledAxisY();
	FVector AxisZ = RotationMatrix.GetScaledAxisZ();

	FVector vDelta(WorldLocation - CameraLocation);
	FVector vTransformed(vDelta | AxisY, vDelta | AxisZ, vDelta | AxisX);

	if (vTransformed.Z == 0.0f)
		vTransformed.Z = -0.001f;

	Screenlocation.Z = vTransformed.Z;

	if (vTransformed.Z < 0.0f)
		vTransformed.Z = -vTransformed.Z;

	float ScreenCenterX = Width / 2.0f;
	float ScreenCenterY = Height / 2.0f;
	float TangentFOV = tanf(ConvertToRadians(CameraFOV / 2.0f));

	Screenlocation.X = ScreenCenterX + vTransformed.X * (ScreenCenterX / TangentFOV) / vTransformed.Z;
	Screenlocation.Y = ScreenCenterY - vTransformed.Y * (ScreenCenterX / TangentFOV) / vTransformed.Z;
	return Screenlocation;
}

bool UWorld::GetUWorld(UWorld& World) {
	NativePtr<EncryptedPtr<UWorld>> PP = g_Pubg->GetBaseAddress() + UWorld::BASE;
	EncryptedPtr<UWorld> P;
	return PP.Read(P) && P.Read(World);
}

FTransform USkeletalMeshSocket::GetSocketLocalTransform() const {
	return FTransform(RelativeRotation, RelativeLocation, RelativeScale);
}

bool USkeleton::FindSocketAndIndex(FName InSocketName, int32& OutIndex, USkeletalMeshSocket& OutSocket) const {
	int Index = -1;
	for (const auto& SocketPtr : Sockets.GetVector()) {
		Index++;

		USkeletalMeshSocket Socket;
		if (!SocketPtr.Read(Socket))
			continue;

		if (Socket.SocketName == InSocketName) {
			OutIndex = Index;
			OutSocket = Socket;
			return true;
		}
	}
	return false;
}

bool USkeletalMesh::FindSocketInfo(FName InSocketName, FTransform& OutTransform, int32& OutBoneIndex, int32& OutIndex, USkeletalMeshSocket& OutSocket) const {
	OutIndex = INDEX_NONE;
	OutTransform = FTransform();
	OutBoneIndex = INDEX_NONE;

	int Index = -1;
	for (const auto& SocketPtr : Sockets.GetVector()) {
		Index++;
		USkeletalMeshSocket Socket;
		if (!SocketPtr.Read(Socket))
			break;

		if (Socket.SocketName == InSocketName) {
			OutIndex = Index;
			OutTransform = Socket.GetSocketLocalTransform();
			OutBoneIndex = FindBoneIndex(Socket.BoneName);
			OutSocket = Socket;
			return true;
		}
	}

	USkeleton Skeleton;
	if (!this->Skeleton.Read(Skeleton))
		return false;

	USkeletalMeshSocket SkeletonSocket;
	if (!Skeleton.FindSocketAndIndex(InSocketName, OutIndex, SkeletonSocket))
		return false;

	OutIndex += (int32)Sockets.size();
	OutTransform = SkeletonSocket.GetSocketLocalTransform();
	OutBoneIndex = FindBoneIndex(SkeletonSocket.BoneName);
	OutSocket = SkeletonSocket;
	return true;
}

int USkeletalMesh::FindBoneIndex(FName BoneName) const {
	int Index = -1;
	for (const FMeshBoneInfo& BoneInfo : FinalRefBoneInfo.GetVector()) {
		Index++;
		if (BoneName == BoneInfo.Name)
			return Index;
	}
	return INDEX_NONE;
}

FName USkeletalMesh::GetBoneName(const int32 BoneIndex) const {
	FMeshBoneInfo BoneInfo;
	if (!FinalRefBoneInfo.GetValue(BoneIndex, BoneInfo))
		return NAME_None;

	return BoneInfo.Name;
}

int32 USkinnedMeshComponent::GetBoneIndex(FName BoneName) const {
	int32 BoneIndex = INDEX_NONE;

	USkeletalMesh SkeletalMesh;
	if (BoneName != NAME_None && this->SkeletalMesh.Read(SkeletalMesh))
		BoneIndex = SkeletalMesh.FindBoneIndex(BoneName);

	return BoneIndex;
}

bool USkinnedMeshComponent::GetSocketInfoByName(FName InSocketName, FTransform& OutTransform, int32& OutBoneIndex, USkeletalMeshSocket& OutSocket) const {
	USkeletalMesh SkeletalMesh;
	if (!this->SkeletalMesh.Read(SkeletalMesh))
		return false;

	int32 SocketIndex;
	return SkeletalMesh.FindSocketInfo(InSocketName, OutTransform, OutBoneIndex, SocketIndex, OutSocket);
}

FTransform USkinnedMeshComponent::GetBoneTransform(int32 BoneIdx, const FTransform& LocalToWorld) const {
	FTransform BoneTransform;
	if (!BoneSpaceTransforms.GetValue(BoneIdx, BoneTransform))
		return FTransform();

	return FTransform(BoneTransform) * LocalToWorld;
}

FTransform USkinnedMeshComponent::GetBoneTransform(int32 BoneIdx) const {
	return GetBoneTransform(BoneIdx, ComponentToWorld);
}

FTransform USkinnedMeshComponent::GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace) const {
	FTransform OutSocketTransform = ComponentToWorld;

	if (InSocketName != NAME_None)
	{
		int32 SocketBoneIndex;
		FTransform SocketLocalTransform;
		USkeletalMeshSocket SkeletalMeshSocket;
		if (GetSocketInfoByName(InSocketName, SocketLocalTransform, SocketBoneIndex, SkeletalMeshSocket)) {
			if (TransformSpace == RTS_ParentBoneSpace)
			{
				//we are done just return now
				return SocketLocalTransform;
			}

			if (SocketBoneIndex != INDEX_NONE)
			{
				FTransform BoneTransform = GetBoneTransform(SocketBoneIndex);
				OutSocketTransform = SocketLocalTransform * BoneTransform;
			}
		}
		else {
			int32 BoneIndex = GetBoneIndex(InSocketName);
			if (BoneIndex != INDEX_NONE)
			{
				OutSocketTransform = GetBoneTransform(BoneIndex);

				if (TransformSpace == RTS_ParentBoneSpace)
				{
					FName ParentBone = GetParentBone(InSocketName);
					int32 ParentIndex = GetBoneIndex(ParentBone);
					if (ParentIndex != INDEX_NONE)
					{
						return OutSocketTransform.GetRelativeTransform(GetBoneTransform(ParentIndex));
					}
					return OutSocketTransform.GetRelativeTransform(ComponentToWorld);
				}
			}
		}
	}

	switch (TransformSpace)
	{
		case RTS_Actor:
		{
			AActor Actor;
			if (Owner.Read(Actor)) {
				return OutSocketTransform.GetRelativeTransform(Actor.ActorToWorld());
			}
			break;
		}
		case RTS_Component:
		{
			return OutSocketTransform.GetRelativeTransform(ComponentToWorld);
		}
	}

	return OutSocketTransform;
}

bool UStaticMesh::FindSocket(FName InSocketName, UStaticMeshSocket& OutSocket) const {
	if (InSocketName == NAME_None)
		return false;

	for (const auto& SocketPtr : Sockets.GetVector()) {
		UStaticMeshSocket Socket;
		if (SocketPtr.Read(Socket) && Socket.SocketName == InSocketName) {
			OutSocket = Socket;
			return true;
		}
	}

	return false;
}

bool UStaticMeshComponent::GetSocketByName(FName InSocketName, UStaticMeshSocket& OutSocket) const {
	UStaticMesh StaticMesh;
	if (!this->StaticMesh.Read(StaticMesh))
		return false;

	return StaticMesh.FindSocket(InSocketName, OutSocket);
}

bool UStaticMeshSocket::GetSocketTransform(FTransform& OutTransform, const UStaticMeshComponent& MeshComp) const {
	OutTransform = FTransform(RelativeRotation, RelativeLocation, RelativeScale) * MeshComp.ComponentToWorld;
	return true;
}

FTransform USceneComponent::GetSocketTransform(FName SocketName, ERelativeTransformSpace TransformSpace) const {
	switch (TransformSpace)
	{
		case RTS_Actor:
		{
			AActor Actor;
			if (Owner.Read(Actor))
				return ComponentToWorld.GetRelativeTransform(Actor.ActorToWorld());
			break;
		}
		case RTS_Component:
		case RTS_ParentBoneSpace:
		{
			return FTransform();
		}
	}
	return ComponentToWorld;
}

FTransform UStaticMeshComponent::GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace) const {
	if (InSocketName != NAME_None) {
		UStaticMeshSocket Socket;
		if (GetSocketByName(InSocketName, Socket))
		{
			FTransform SocketWorldTransform;
			if (Socket.GetSocketTransform(SocketWorldTransform, *this))
			{
				switch (TransformSpace)
				{
					case RTS_World:
					{
						return SocketWorldTransform;
					}
					case RTS_Actor:
					{
						AActor Actor;
						if (Owner.Read(Actor)) {
							return SocketWorldTransform.GetRelativeTransform(Actor.ActorToWorld());
						}
						break;
					}
					case RTS_Component:
					{
						return SocketWorldTransform.GetRelativeTransform(ComponentToWorld);
					}
				}
			}
		}
	}
		
	return USceneComponent::GetSocketTransform(InSocketName, TransformSpace);
}

FTransform AActor::ActorToWorld() const {
	USceneComponent SceneComponent;
	if (!RootComponent.Read(SceneComponent))
		return FTransform();

	return SceneComponent.ComponentToWorld;
}


FName USkinnedMeshComponent::GetParentBone(FName BoneName) const {
	int32 BoneIndex = GetBoneIndex(BoneName);
	if (BoneIndex == INDEX_NONE || BoneIndex == 0)
		return NAME_None;

	USkeletalMesh SkeletalMesh;
	if (!this->SkeletalMesh.Read(SkeletalMesh))
		return NAME_None;

	return SkeletalMesh.GetBoneName(SkeletalMesh.GetParentIndex(BoneIndex));
}

int32 USkeletalMesh::GetParentIndex(const int32 BoneIndex) const {
	FMeshBoneInfo BoneInfo;
	if (!FinalRefBoneInfo.GetValue(BoneIndex, BoneInfo))
		return INDEX_NONE;

	return BoneInfo.ParentIndex;
}
#pragma once
#include "ue4math.h"

struct FRotator;

// ScriptStruct CoreUObject.Vector
// 0x000C
struct FVector
{
public:
	float                                              X;                                                        // 0x0000(0x0004) (CPF_Edit, CPF_BlueprintVisible, CPF_ZeroConstructor, CPF_SaveGame, CPF_IsPlainOldData)
	float                                              Y;                                                        // 0x0004(0x0004) (CPF_Edit, CPF_BlueprintVisible, CPF_ZeroConstructor, CPF_SaveGame, CPF_IsPlainOldData)
	float                                              Z;                                                        // 0x0008(0x0004) (CPF_Edit, CPF_BlueprintVisible, CPF_ZeroConstructor, CPF_SaveGame, CPF_IsPlainOldData)

	FVector() : X(0.0), Y(0.0), Z(0.0) {}
	FVector(float X, float Y, float Z) :X(X), Y(Y), Z(Z) {}

	float DotProduct(const FVector& v) const {
		return (X * v.X) + (Y * v.Y) + (Z * v.Z);
	}

	FVector CrossProduct(const FVector& v) const {
		FVector output;
		output.X = (Y * v.Z) - (Z * v.Y);
		output.Y = (Z * v.X) - (X * v.Z);
		output.Z = (X * v.Y) - (Y * v.X);
		return output;
	}

	FVector Min(const FVector& v) const {
		FVector output;
		output.X = X < v.X ? X : v.X;
		output.Y = Y < v.Y ? Y : v.Y;
		output.Z = Z < v.Z ? Z : v.Z;
		return output;
	}

	FVector Max(const FVector& v) const {
		FVector output;
		output.X = X > v.X ? X : v.X;
		output.Y = Y > v.Y ? Y : v.Y;
		output.Z = Z > v.Z ? Z : v.Z;
		return output;
	}

	bool operator == (const FVector& v) const {
		return X == v.X && Y == v.Y && Z == v.Z;
	}

	bool operator != (const FVector& v) const {
		return !(*this == v);
	}

	FVector operator - () const {
		return FVector(-X, -Y, -Z);
	}

	FVector operator + (const FVector& v) const {
		return FVector(X + v.X, Y + v.Y, Z + v.Z);
	}

	FVector operator - (const FVector& v) const {
		return FVector(X - v.X, Y - v.Y, Z - v.Z);
	}

	FVector operator * (const FVector& v) const {
		return FVector(X * v.X, Y * v.Y, Z * v.Z);
	}

	FVector operator * (float Value) const {
		return FVector(X * Value, Y * Value, Z * Value);
	}

	FVector GetNormalizedVector() const {
		return operator*(1.0f / sqrtf(X * X + Y * Y + Z * Z));
	}

	void Normalize() {
		*this = GetNormalizedVector();
	}

	float Length() const {
		return sqrtf(X * X + Y * Y + Z * Z);
	}

	float Distance(const FVector& v) const {
		return (v - *this).Length();
	}

	FVector operator ^ (const FVector& v) const {
		return CrossProduct(v);
	}

	float operator | (const FVector& v) const {
		return DotProduct(v);
	}

	FVector GetSignVector() const
	{
		return FVector
		(
			Select(X, 1.0, -1.0),
			Select(Y, 1.0, -1.0),
			Select(Z, 1.0, -1.0)
		);
	}

	bool IsNearlyZero(float Tolerance = KINDA_SMALL_NUMBER) const {
		return fabs(X) <= Tolerance && fabs(Y) <= Tolerance && fabs(Z) <= Tolerance;
	}

	FRotator GetDirectionRotator() const;
};

static FVector operator * (float Value, const FVector& v) {
	return v.operator*(Value);
}

static_assert(sizeof(FVector) == 0xC, "FVector");
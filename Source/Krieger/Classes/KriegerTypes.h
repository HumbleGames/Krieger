// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "KriegerTypes.generated.h"

#pragma once

UENUM(BlueprintType)
namespace EKriegerPhysMaterialType
{
	enum Type
	{
		Unknown,
		Concrete,
		Dirt,
		Water,
		Metal,
		Wood,
		Grass,
		Glass,
		Flesh,
	};
}

#define KRIEGER_SURFACE_Default		SurfaceType_Default
#define KRIEGER_SURFACE_Concrete	SurfaceType1
#define KRIEGER_SURFACE_Dirt		SurfaceType2
#define KRIEGER_SURFACE_Water		SurfaceType3
#define KRIEGER_SURFACE_Metal		SurfaceType4
#define KRIEGER_SURFACE_Wood		SurfaceType5
#define KRIEGER_SURFACE_Grass		SurfaceType6
#define KRIEGER_SURFACE_Glass		SurfaceType7
#define KRIEGER_SURFACE_Flesh		SurfaceType8

/**
 * Decal config for effects
 */
USTRUCT(BlueprintType)
struct FDecalData
{
	GENERATED_USTRUCT_BODY()

	/** Material */
	UPROPERTY(EditDefaultsOnly, Category = Decal)
	UMaterial* DecalMaterial;

	/** Quad size (width & height) */
	UPROPERTY(EditDefaultsOnly, Category = Decal)
	float DecalSize;

	/** Lifespan */
	UPROPERTY(EditDefaultsOnly, Category = Decal)
	float LifeSpan;

	/** Defaults */
	FDecalData()
		: DecalSize(256.f)
		, LifeSpan(10.f)
	{
	}
};

/**
 * Weapon barrel config
 */
USTRUCT(BlueprintType)
struct FWeaponBarrel
{
	GENERATED_USTRUCT_BODY()

	/** Name of bone/socket for muzzle in weapon mesh */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	FName MuzzleAttachPoint;

	/** Spawned component for muzzle FX */
	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSC;

	/** Defaults */
	FWeaponBarrel()
	{
		MuzzleAttachPoint = TEXT("");
		MuzzlePSC = NULL;
	}
};

/**
 * Projectile config
 */
USTRUCT(BlueprintType)
struct FProjectileWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** Projectile class */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class AKriegerProjectile> ProjectileClass;

	/** Life time */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	float ProjectileLife;

	/** Damage at impact point */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	int32 ExplosionDamage;

	/** Radius of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float ExplosionRadius;

	/** Type of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	TSubclassOf<UDamageType> DamageType;

	/** Defaults */
	FProjectileWeaponData()
	{
		ProjectileClass = NULL;
		ProjectileLife = 10.0f;
		ExplosionDamage = 100;
		ExplosionRadius = 300.0f;
		DamageType = UDamageType::StaticClass();
	}
};

USTRUCT(BlueprintType)
struct FInstantHitInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector Origin;

	UPROPERTY()
	float ReticleSpread;

	UPROPERTY()
	int32 RandomSeed;
};

USTRUCT(BlueprintType)
struct FInstantWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** Base weapon spread (degrees) */
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float WeaponSpread;

	/** Targeting spread modifier */
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float TargetingSpreadMod;

	/** Continuous firing: spread increment */
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float FiringSpreadIncrement;

	/** Continuous firing: max increment */
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float FiringSpreadMax;

	/** Weapon range */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	float WeaponRange;

	/** Damage amount */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	int32 HitDamage;

	/** Type of damage */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	TSubclassOf<UDamageType> DamageType;

	/** Hit verification: scale for bounding box of hit actor */
	UPROPERTY(EditDefaultsOnly, Category=HitVerification)
	float ClientSideHitLeeway;

	/** Hit verification: threshold for dot product between view direction and hit direction */
	UPROPERTY(EditDefaultsOnly, Category=HitVerification)
	float AllowedViewDotHitDir;

	/** Defaults */
	FInstantWeaponData()
	{
		WeaponSpread = 5.0f;
		TargetingSpreadMod = 0.25f;
		FiringSpreadIncrement = 1.0f;
		FiringSpreadMax = 10.0f;
		WeaponRange = 10000.0f;
		HitDamage = 10;
		DamageType = UDamageType::StaticClass();
		ClientSideHitLeeway = 200.0f;
		AllowedViewDotHitDir = 0.8f;
	}
};

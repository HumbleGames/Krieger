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
USTRUCT()
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

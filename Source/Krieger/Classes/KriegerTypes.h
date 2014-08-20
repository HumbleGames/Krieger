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

UENUM(BlueprintType)
namespace EWeaponType
{
	enum Type
	{
		InstantHit,
		Projectile
	};
}

USTRUCT(BlueprintType)
struct FWeaponMode
{
	GENERATED_USTRUCT_BODY()

	
	//////////////////////////////////////////////////////////////////////////
	// GENERAL CONFIG

	/** How weapon hadles fire */
	UPROPERTY(EditDefaultsOnly, Category = General)
	TEnumAsByte<EWeaponType::Type> WeaponType;

	/** Number of bullets in burst */
	UPROPERTY(EditDefaultsOnly, Category = General)
	int32 BulletsPerShot;

	/** Shot all bullets at the same time or not? */
	UPROPERTY(EditDefaultsOnly, Category = General)
	bool ShotBulletsSimultaneously;

	/** Ammo use for burst. If == -1 bullets number in burst will be used */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	int32 AmmoPerShot;

	/** Time between two consecutive bursts */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	float TimeBetweenBursts;

	/** Time between two shots in burst */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float TimeBetweenShots;


	//////////////////////////////////////////////////////////////////////////
	// Effects
	
	/** Firing audio (bLoopedFireSound set) */
	UPROPERTY(Transient)
	UAudioComponent* FireAC;

	/** Weapon barrels */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TArray<FWeaponBarrel> WeaponBarrels;

	/** FX for muzzle flash */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* MuzzleFX;

	/** Camera shake on firing */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	TSubclassOf<UCameraShake> FireCameraShake;

	/** Force feedback effect to play when the weapon is fired */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UForceFeedbackEffect *FireForceFeedback;

	/** Single fire sound (bLoopedFireSound not set) */
	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* FireSound;

	/** Looped fire sound (bLoopedFireSound set) */
	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* FireLoopSound;

	/** Finished burst sound (bLoopedFireSound set) */
	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* FireFinishSound;

	/** Fire animations */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	UAnimMontage* FireAnim;

	/** Is muzzle FX looped? */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	uint32 bLoopedMuzzleFX : 1;

	/** Is muzzle FX played simultaneously? */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	uint32 bSimultaneousMuzzleFX : 1;

	/** Is fire sound looped? */
	UPROPERTY(EditDefaultsOnly, Category=Sound)
	uint32 bLoopedFireSound : 1;

	/** Is fire animation looped? */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	uint32 bLoopedFireAnim : 1;


	//////////////////////////////////////////////////////////////////////////
	// INSTANT HIT WEAPON

	/** Weapon config */
	UPROPERTY(EditDefaultsOnly, Category=ProjectileWeapon)
	FProjectileWeaponData ProjectileConfig;


	//////////////////////////////////////////////////////////////////////////
	// INSTANT HIT WEAPON

	/** Instant weapon config */
	UPROPERTY(EditDefaultsOnly, Category=InstantHitWeapon)
	FInstantWeaponData InstantConfig;

	/** Impact effects */
	UPROPERTY(EditDefaultsOnly, Category=InstantHitWeapon)
	TSubclassOf<class AKriegerImpactEffect> ImpactTemplate;

	/** Smoke trail for instant hit weapon */
	UPROPERTY(EditDefaultsOnly, Category=InstantHitWeapon)
	UParticleSystem* TrailFX;

	/** Param name for beam target in smoke trail */
	UPROPERTY(EditDefaultsOnly, Category=InstantHitWeapon)
	FName TrailTargetParam;


	//////////////////////////////////////////////////////////////////////////
	// DEFAULTS

	FWeaponMode()
	{
		BulletsPerShot = 1;
		ShotBulletsSimultaneously = false;
		AmmoPerShot = -1;
		TimeBetweenBursts = 0.2f;
		TimeBetweenShots = 0.2f;

		bLoopedMuzzleFX = false;
		bLoopedFireAnim = false;
	}

};

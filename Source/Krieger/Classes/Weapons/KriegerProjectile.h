// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "KriegerProjectile.generated.h"

UCLASS(Abstract, Blueprintable, DependsOn=AKriegerWeapon_Projectile)
class AKriegerProjectile : public AActor
{
	GENERATED_UCLASS_BODY()

	/** Initial setup */
	virtual void PostInitializeComponents() OVERRIDE;

	/** Setup velocity */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon|Projectile")
	void InitVelocity(FVector ShootDirection);

	/** Apply weapon config */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon|Projectile")
	void SetWeaponConfig(const FProjectileWeaponData& Data);

	/** Handle hit */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon|Projectile")
	void OnImpact(const FHitResult& HitResult);

protected:

	/** Movement component */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	TSubobjectPtr<UProjectileMovementComponent> MovementComp;

	/** Collisions */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	TSubobjectPtr<USphereComponent> CollisionComp;

	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	TSubobjectPtr<UParticleSystemComponent> ParticleComp;

	/** Effects for explosion */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	TSubclassOf<class AKriegerExplosionEffect> ExplosionTemplate;

	/** Controller that fired me (cache for damage calculations) */
	UPROPERTY()
	TWeakObjectPtr<AController> MyController;

	/** Projectile data */
	UPROPERTY(Transient)
	struct FProjectileWeaponData WeaponConfig;

	/** Did it explode? */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_Exploded)
	bool bExploded;

	/** [client] explosion happened */
	UFUNCTION()
	void OnRep_Exploded();

	/** Trigger explosion */
	void Explode(const FHitResult& Impact);

	/** Shutdown projectile and prepare for destruction */
	void DisableAndDestroy();

	/** Update velocity on client */
	virtual void PostNetReceiveVelocity(const FVector& NewVelocity) OVERRIDE;

};

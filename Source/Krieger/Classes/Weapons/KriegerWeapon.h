// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.
// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "KriegerWeapon.generated.h"

UENUM(BlueprintType)
namespace EWeaponState
{
	enum Type
	{
		Idle,
		Firing,
		Reloading,
		Equipping,
	};
}

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOutOfAmmo);

UCLASS(Abstract, Blueprintable)
class AKriegerWeapon : public AActor
{
	GENERATED_UCLASS_BODY()

	/** Initial setup */
	virtual void PostInitializeComponents() override;
	
	/** Cleanup */
	virtual void Destroyed() override;


	//////////////////////////////////////////////////////////////////////////
	// General config

protected:
	/** Primary weapon mode */
	UPROPERTY(EditDefaultsOnly, Category = General)
	FWeaponMode PrimaryFireMode;

	/** Is Secondary weapon fire mode enabled? */
	UPROPERTY(EditDefaultsOnly, Category = General)
	bool bSecondaryFireModeEnabled;

	/** Secondary weapon mode */
	UPROPERTY(EditDefaultsOnly, Category = General, Meta = (EditCondition = "bSecondaryFireModeEnabled"))
	FWeaponMode SecondaryFireMode;

	/** Inifite ammo for reloads */
	UPROPERTY(EditDefaultsOnly, Category=Ammo)
	bool bInfiniteAmmo;

	/** Infinite ammo in clip, no reload required */
	UPROPERTY(EditDefaultsOnly, Category=Ammo)
	bool bInfiniteClip;

	/** Max ammo */
	UPROPERTY(EditDefaultsOnly, Category=Ammo)
	int32 MaxAmmo;

	/** Clip size */
	UPROPERTY(EditDefaultsOnly, Category=Ammo)
	int32 AmmoPerClip;

	/** Initial clips */
	UPROPERTY(EditDefaultsOnly, Category=Ammo)
	int32 InitialClips;

	/** Failsafe reload duration if weapon doesn't have any animation for it */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	float NoAnimReloadDuration;


	//////////////////////////////////////////////////////////////////////////
	// Visuals

protected:
	/** Pawn owner */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_MyPawn)
	class AKriegerCharacter* MyPawn;

	/** Weapon mesh */
	UPROPERTY(VisibleAnywhere, Category=Mesh)
	TSubobjectPtr<USkeletalMeshComponent> Mesh;

	/** Out of ammo sound */
	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* OutOfAmmoSound;

	/** Reload sound */
	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* ReloadSound;

	/** Reload animations */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	UAnimMontage* ReloadAnim;

	/** Equip sound */
	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* EquipSound;

	/** Equip animations */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	UAnimMontage* EquipAnim;


	//////////////////////////////////////////////////////////////////////////
	// Ammo

public:
	/** [server] Add ammo */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	void GiveAmmo(int32 AddAmount);

	/** Consume a bullet */
	void UseAmmo();


	//////////////////////////////////////////////////////////////////////////
	// Inventory

	/** Weapon is being equipped by owner pawn */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	virtual void OnEquip();

	/** Weapon is now equipped by owner pawn */
	virtual void OnEquipFinished();

	/** Weapon is holstered by owner pawn */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	virtual void OnUnEquip();

	/** [server] Weapon was added to pawn's inventory */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	virtual void OnEnterInventory(AKriegerCharacter* NewOwner);

	/** [server] Weapon was removed from pawn's inventory */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	virtual void OnLeaveInventory();

	/** Check if it's currently equipped */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	bool IsEquipped() const;

	/** Check if mesh is already attached */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	bool IsAttachedToPawn() const;


	//////////////////////////////////////////////////////////////////////////
	// Mesh

protected:
	/** Attach weapon mesh to pawn's mesh */
	virtual void AttachMeshToPawn();

	/** Detach weapon mesh from pawn */
	void DetachMeshFromPawn();


	//////////////////////////////////////////////////////////////////////////
	// Input

public:
	/** [local + server] Start weapon fire */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	virtual void StartFire(uint32 WeaponMode);

	/** [local + server] Stop weapon fire */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	virtual void StopFire();

	/** [all] Start weapon reload */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	virtual void StartReload(bool bFromReplication = false);

	/** [local + server] Interrupt weapon reload */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	virtual void StopReload();

	/** [server] Performs actual reload */
	virtual void ReloadWeapon();

	/** Trigger reload from server */
	UFUNCTION(reliable, client)
	void ClientStartReload();


	//////////////////////////////////////////////////////////////////////////
	// Control

	/** Check if weapon can fire */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	bool CanFire() const;

	/** Check if weapon can be reloaded */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	bool CanReload() const;


	//////////////////////////////////////////////////////////////////////////
	// Reading data

	/** Get current weapon state */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	EWeaponState::Type GetCurrentState() const;

	/** Get current ammo amount (total) */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	int32 GetCurrentAmmo() const;

	/** Get current ammo amount (clip) */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	int32 GetCurrentAmmoInClip() const;

	/** Get clip size */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	int32 GetAmmoPerClip() const;

	/** Get max ammo amount */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	int32 GetMaxAmmo() const;

	/** Get weapon mesh (needs pawn owner to determine variant) */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	USkeletalMeshComponent* GetWeaponMesh() const;

	/** Get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	class AKriegerCharacter* GetPawnOwner() const;

	/** Check if weapon has infinite ammo (include owner's cheats) */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	bool HasInfiniteAmmo() const;

	/** Check if weapon has infinite clip (include owner's cheats) */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	bool HasInfiniteClip() const;

	/** Set the weapon's owning pawn */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	void SetOwningPawn(AKriegerCharacter* NewOwner);

	/** Gets last time when this weapon was switched to */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	float GetEquipStartedTime() const;

	/** Gets the duration of equipping weapon */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Weapon")
	float GetEquipDuration() const;


	//////////////////////////////////////////////////////////////////////////
	// Notification events

	/** Called when weapon has no ammo to shoot */
	UPROPERTY(BlueprintAssignable)
	FOutOfAmmo OutOfAmmo;

protected:
	//////////////////////////////////////////////////////////////////////////
	// Internal data

	/** Is fire animation playing? */
	uint32 bPlayingFireAnim : 1;

	/** Is weapon currently equipped? */
	uint32 bIsEquipped : 1;

	/** Is weapon fire active? */
	uint32 bWantsToFire : 1;

	/** Is reload animation playing? */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_Reload)
	uint32 bPendingReload : 1;

	/** Is equip animation playing? */
	uint32 bPendingEquip : 1;

	/** Weapon is refiring */
	uint32 bRefiring;

	/** Current weapon state */
	EWeaponState::Type CurrentState;

	/** Current fire mode */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentFireMode)
	uint32 CurrentFireMode;

	/** Current fire mode */
	FWeaponMode* CurrentWeaponFireMode;

	/** Time of last successful weapon fire */
	float LastFireTime;

	/** Last time when this weapon was switched to */
	float EquipStartedTime;

	/** How much time weapon needs to be equipped */
	float EquipDuration;

	/** Current total ammo */
	UPROPERTY(Transient, Replicated)
	int32 CurrentAmmo;

	/** Current ammo - inside clip */
	UPROPERTY(Transient, Replicated)
	int32 CurrentAmmoInClip;

	/** Burst counter, used for replicating fire events to remote clients */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_BurstCounter)
	int32 BurstCounter;

	/** Current active barrel */
	UPROPERTY(Transient, Replicated)
	int32 CurrentBarrel;


	//////////////////////////////////////////////////////////////////////////
	// Input - server side

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartFire(uint32 WeaponMode);

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopFire();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartReload();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopReload();


	//////////////////////////////////////////////////////////////////////////
	// Replication & effects

	UFUNCTION()
	void OnRep_MyPawn();

	UFUNCTION()
	void OnRep_BurstCounter();

	UFUNCTION()
	void OnRep_Reload();

	UFUNCTION()
	void OnRep_CurrentFireMode();

	/** Called in network play to do the cosmetic fx for firing */
	virtual void SimulateWeaponFire();

	/** Called in network play to stop cosmetic fx (e.g. for a looping shot). */
	virtual void StopSimulatingWeaponFire();


	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** [local] Weapon specific fire implementation */
	virtual void FireWeapon();

	/** [local] Instant hit weapon fire */
	virtual void FireWeapon_Instant();

	/** [local] Projectile weapon fire */
	virtual void FireWeapon_Projectile();

	/** [server] Fire & update ammo */
	UFUNCTION(reliable, server, WithValidation)
	void ServerHandleFiring();

	/** [local + server] Handle weapon fire */
	void HandleFiring();

	/** [local + server] Firing started */
	virtual void OnBurstStarted();

	/** [local + server] Firing finished */
	virtual void OnBurstFinished();

	/** Update weapon state */
	void SetWeaponState(EWeaponState::Type NewState);

	/** Determine current weapon state */
	void DetermineWeaponState();

	/** Determine current weapon mode */
	void DetermineWeaponMode();


	//////////////////////////////////////////////////////////////////////////
	// Weapon usage helpers

	/** Get current weapon mode */
	FWeaponMode* GetCurrentWeaponMode() const;

	/** Play weapon sounds */
	UAudioComponent* PlayWeaponSound(USoundCue* Sound);

	/** Play weapon animations */
	float PlayWeaponAnimation(UAnimMontage* Animation);

	/** Stop playing weapon animations */
	void StopWeaponAnimation(UAnimMontage* Animation);

	/** Get the point we're targeting too */
	virtual FVector GetTargetPoint() const;

	/** Get the aim of the weapon, allowing for adjustments to be made by the weapon */
	virtual FVector GetAdjustedAim() const;

	/** Get the location to fire weapon */
	virtual FVector GetDamageStartLocation() const;

	/** Get the muzzle location of the weapon */
	FVector GetMuzzleLocation() const;

	/** Get direction of weapon's muzzle */
	FVector GetMuzzleDirection() const;

	/** Get current barrel attach point */
	FName GetCurrentBarrelSocketName() const;

	/** Find hit */
	FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const;



	//////////////////////////////////////////////////////////////////////////
	// INSTANT HIT WEAPON

public:
	/** Get current spread */
	float GetCurrentSpread() const;

protected:
	/** Instant hit notify for replication */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_HitNotify)
	FInstantHitInfo HitNotify;

	/** Current spread from continuous firing */
	float CurrentFiringSpread;

	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** Server notified of hit from client to verify */
	UFUNCTION(reliable, server, WithValidation)
	void ServerNotifyHit(const FHitResult Impact, FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);

	/** Server notified of miss to show trail FX */
	UFUNCTION(unreliable, server, WithValidation)
	void ServerNotifyMiss(FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);

	/** Process the instant hit and notify the server if necessary */
	void ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);

	/** Continue processing the instant hit, as if it has been confirmed by the server */
	void ProcessInstantHit_Confirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);

	/** Check if weapon should deal damage to actor */
	bool ShouldDealDamage(AActor* TestActor) const;

	/** Handle damage */
	void DealDamage(const FHitResult& Impact, const FVector& ShootDir);


	//////////////////////////////////////////////////////////////////////////
	// Effects replication
	
	UFUNCTION()
	void OnRep_HitNotify();

	/** Called in network play to do the cosmetic fx  */
	void SimulateInstantHit(const FVector& Origin, int32 RandomSeed, float ReticleSpread);

	/** Spawn effects for impact */
	void SpawnImpactEffects(const FHitResult& Impact);

	/** Spawn trail effect */
	void SpawnTrailEffect(const FVector& EndPoint);



	//////////////////////////////////////////////////////////////////////////
	// PROJECTILE WEAPON

public:
	/** Apply config on projectile */
	void ApplyWeaponConfig(FProjectileWeaponData& Data);

protected:
	/** Spawn projectile on server */
	UFUNCTION(reliable, server, WithValidation)
	void ServerFireProjectile(FVector Origin, FVector_NetQuantizeNormal ShootDir);

};


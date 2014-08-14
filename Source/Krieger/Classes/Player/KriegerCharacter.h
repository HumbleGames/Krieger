// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "KriegerCharacter.generated.h"

UCLASS(Abstract, Blueprintable)
class AKriegerCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

	/** Spawn inventory, setup initial variables */
	virtual void PostInitializeComponents() override;

	/** Cleanup inventory */
	virtual void Destroyed() override;

	/** [server] Perform PlayerState related setup */
	virtual void PossessedBy(class AController* C) override;

	/** [client] Perform PlayerState related setup */
	virtual void OnRep_PlayerState() override;


	//////////////////////////////////////////////////////////////////////////
	// Health

	/** Check if pawn is still alive */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Character")
	bool IsAlive() const;

	/** Get current health */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Character")
	float GetHealth() const;

	/** [server] Set current health */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Character")
	void SetHealth(float NewHealth);

	/** Get max health */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Character")
	float GetMaxHealth() const;

	/** [server] Set max health */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Character")
	void SetMaxHealth(float NewMaxHealth);

protected:
	/** Current health of the Pawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category=Health)
	float Health;

	/** Maximum health of the Pawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category=Health)
	float MaxHealth;


	//////////////////////////////////////////////////////////////////////////
	// Inventory
protected:

	/** [server] Spawns default inventory */
	void SpawnDefaultInventory();

	/** [server] Remove all weapons from inventory and destroy them */
	void DestroyInventory();


	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

public:
	/** [local] starts weapon fire */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Character")
	void StartFire();

	/** [local] stops weapon fire */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Character")
	void StopFire();

protected:
	/** [local] Character specific fire implementation */
	virtual void StartWeaponFire() PURE_VIRTUAL(AKriegerCharacter::StartWeaponFire, );
	virtual void StopWeaponFire() PURE_VIRTUAL(AKriegerCharacter::StopWeaponFire, );

public:
	/** Check if pawn can fire weapon */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Character")
	bool CanFire() const;

	/** Check if pawn can reload weapon */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Character")
	bool CanReload() const;

	/** Get firing state */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Character")
	bool IsFiring() const;

protected:
	/** Current firing state */
	uint8 bWantsToFire : 1;


	//////////////////////////////////////////////////////////////////////////
	// Meshes

public:
	/** Update the team color of all player meshes. */
	void UpdateTeamColorsAllMIDs();

protected:
	/** Handle mesh colors on specified material instance */
	void UpdateTeamColors(UMaterialInstanceDynamic* UseMID);

	/** Material instances for setting team color in mesh (3rd person view) */
	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic*> MeshMIDs;


	//////////////////////////////////////////////////////////////////////////
	// Effects

protected:
	/** Effect played on respawn */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UParticleSystem* RespawnFX;


	//////////////////////////////////////////////////////////////////////////
	// Sound

protected:
	/** Sound played on death, local player only */
	UPROPERTY(EditDefaultsOnly, Category=Pawn)
	USoundCue* DeathSound;

	/** Sound played on respawn */
	UPROPERTY(EditDefaultsOnly, Category=Pawn)
	USoundCue* RespawnSound;

};


// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "KriegerMech.generated.h"

class AKriegerWeapon;
class AKriegerJetpack;

UCLASS(Abstract, Blueprintable)
class AKriegerMech : public AKriegerCharacter
{
	GENERATED_UCLASS_BODY()


	//////////////////////////////////////////////////////////////////////////
	// Inventory
protected:

	/** [server] Spawns default inventory */
	virtual void SpawnDefaultInventory() override;

	/** [server] Remove all weapons from inventory and destroy them */
	virtual void DestroyInventory() override;

	/** [server] Spawn and add weapon to inventory */
	void AddWeapon(AKriegerWeapon* Weapon);

	/** [server] Remove weapon from inventory and destroy */
	void RemoveWeapon(AKriegerWeapon* Weapon);

	/** Get weapon attach point */
	virtual FName GetWeaponAttachPoint(AKriegerWeapon* Weapon) const override;


	//////////////////////////////////////////////////////////////////////////
	// Weapons (Usage)

	/** Get total number of inventory items */
	int32 GetInventoryCount() const;

	/** Get weapon from inventory at index. Index validity is not checked */
	AKriegerWeapon* GetInventoryWeapon(int32 index) const;

	/** 
	 * [server + local] equips weapon from inventory 
	 *
	 * @param Weapon	Weapon to equip
	 */
	void EquipWeapon(AKriegerWeapon** WeaponSlot, AKriegerWeapon* Weapon);

protected:
	/** Updates current weapon in slot */
	void SetCurrentWeapon(AKriegerWeapon** WeaponSlot, AKriegerWeapon* NewWeapon, AKriegerWeapon* LastWeapon = NULL);


	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

protected:
	/** [local] Character specific fire implementation */
	virtual void StartWeaponFire(int32 WeaponIdx) override;
	virtual void StopWeaponFire(int32 WeaponIdx) override;


	//////////////////////////////////////////////////////////////////////////
	// Modules (Usage)

	/** [local] Start Jetpack "fire" */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Mech")
	void ActivateJetpack();

	/** [local] Stop Jetpack "fire" */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Mech")
	void DeactivateJetpack();


	//////////////////////////////////////////////////////////////////////////
	// Weapons (Data)

protected:
	/** Socket or bone name for attaching weapon mesh (Right arm) */
	UPROPERTY(EditDefaultsOnly, Category = Weapons)
	FName WeaponSocketRight;

	/** Socket or bone name for attaching weapon mesh (Left arm) */
	UPROPERTY(EditDefaultsOnly, Category = Weapons)
	FName WeaponSocketLeft;

	/** Socket or bone name for attaching weapon mesh (Body) */
	UPROPERTY(EditDefaultsOnly, Category = Weapons)
	FName WeaponSocketBody;

	/** Socket or bone name for attaching weapon mesh (Back; Heavy mech only) */
	UPROPERTY(EditDefaultsOnly, Category = Weapons)
	FName WeaponSocketBack;

	/** Default weapon (Right arm) */
	UPROPERTY(EditDefaultsOnly, Category = Weapons)
	TSubclassOf<AKriegerWeapon> DefaultWeaponRight;

	/** Default weapon (Left arm) */
	UPROPERTY(EditDefaultsOnly, Category = Weapons)
	TSubclassOf<AKriegerWeapon> DefaultWeaponLeft;

	/** Default weapon (Body) */
	UPROPERTY(EditDefaultsOnly, Category = Weapons)
	TSubclassOf<AKriegerWeapon> DefaultWeaponBody;

	/** Default weapon (Back; Heavy mech only) */
	UPROPERTY(EditDefaultsOnly, Category = Weapons)
	TSubclassOf<AKriegerWeapon> DefaultWeaponBack;

	/** Currently equipped weapon (Right arm) */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_WeaponRight)
	AKriegerWeapon* WeaponRight;

	/** Currently equipped weapon (Left arm) */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_WeaponLeft)
	AKriegerWeapon* WeaponLeft;

	/** Currently equipped weapon (Body) */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_WeaponBody)
	AKriegerWeapon* WeaponBody;

	/** Currently equipped weapon (Back) */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_WeaponBack)
	AKriegerWeapon* WeaponBack;


	//////////////////////////////////////////////////////////////////////////
	// Modules

	/** Socket or bone name for attaching Jetpack mesh */
	UPROPERTY(EditDefaultsOnly, Category = Modules)
	FName JetpackSocketBack;

	/** Default Jetpack */
	UPROPERTY(EditDefaultsOnly, Category = Modules)
	TSubclassOf<AKriegerJetpack> DefaultJetpack;

	/** Currently equipped Jetpack */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Jetpack)
	AKriegerWeapon* Jetpack;


	//////////////////////////////////////////////////////////////////////////
	// Inventory

	/** All weapons in inventory */
	UPROPERTY(Transient, Replicated)
	TArray<AKriegerWeapon*> WeaponInventory;


	//////////////////////////////////////////////////////////////////////////
	// Replication

protected:
	/** Current weapon replication handler (Right arm) */
	UFUNCTION()
	void OnRep_WeaponRight(AKriegerWeapon* LastWeapon);

	/** Current weapon replication handler (Left arm) */
	UFUNCTION()
	void OnRep_WeaponLeft(AKriegerWeapon* LastWeapon);

	/** Current weapon replication handler (Body) */
	UFUNCTION()
	void OnRep_WeaponBody(AKriegerWeapon* LastWeapon);

	/** Current weapon replication handler (Back) */
	UFUNCTION()
	void OnRep_WeaponBack(AKriegerWeapon* LastWeapon);

	/** Current Jetpack replication handler */
	UFUNCTION()
	void OnRep_Jetpack(AKriegerWeapon* LastJetpack);


};


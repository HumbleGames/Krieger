// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "KriegerCharacter.generated.h"

UCLASS(Abstract, Blueprintable)
class AKriegerCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

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
	// Weapon usage

public:
	/** Check if pawn can fire weapon */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Character")
	bool CanFire() const;

	/** Check if pawn can reload weapon */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Character")
	bool CanReload() const;

};


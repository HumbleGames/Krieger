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

	/** Get max health */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Character")
	int32 GetMaxHealth() const;

	/** Current health of the Pawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category=Health)
	float Health;


	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** Check if pawn can fire weapon */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Character")
	bool CanFire() const;

	/** Check if pawn can reload weapon */
	UFUNCTION(BlueprintCallable, Category = "Krieger|Character")
	bool CanReload() const;

};


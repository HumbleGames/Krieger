// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerController.h"
#include "KriegerPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class AKriegerPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

	/** Initialize the input system from the player settings */
	virtual void InitInputSystem() OVERRIDE;

	/** Local function to change system settings and apply them */
	UFUNCTION(exec, BlueprintCallable, Category = "Settings")
	void SetGraphicsQuality(int32 InGraphicsQuality);

};

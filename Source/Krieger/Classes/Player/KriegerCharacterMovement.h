// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#pragma once
#include "KriegerCharacterMovement.generated.h"

UCLASS()
class UKriegerCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()

	/** Max speed depends on character state */
	virtual float GetMaxSpeed() const override;

};

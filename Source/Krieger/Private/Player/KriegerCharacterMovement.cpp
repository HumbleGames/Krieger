// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "Krieger.h"

UKriegerCharacterMovement::UKriegerCharacterMovement(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
}

float UKriegerCharacterMovement::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const AKriegerCharacter* KriegerCharacterOwner = Cast<AKriegerCharacter>(PawnOwner);
	if (KriegerCharacterOwner)
	{
		// @TODO Change max speed based on character walk state
	}

	return MaxSpeed;
}

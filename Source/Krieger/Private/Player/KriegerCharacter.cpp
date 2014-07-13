// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "Krieger.h"

AKriegerCharacter::AKriegerCharacter(const class FPostConstructInitializeProperties& PCIP) 
	: Super(PCIP)
{
	Health = 100;
}

bool AKriegerCharacter::IsAlive() const
{
	return Health > 0;
}

float AKriegerCharacter::GetHealth() const
{
	return Health;
}

int32 AKriegerCharacter::GetMaxHealth() const
{
	return GetClass()->GetDefaultObject<AKriegerCharacter>()->Health;
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage

bool AKriegerCharacter::CanFire() const
{
	return IsAlive();
}

bool AKriegerCharacter::CanReload() const
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
// Replication

void AKriegerCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AKriegerCharacter, Health);
}

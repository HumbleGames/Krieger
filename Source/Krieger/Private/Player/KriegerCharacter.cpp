// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "Krieger.h"

AKriegerCharacter::AKriegerCharacter(const class FPostConstructInitializeProperties& PCIP) 
	: Super(PCIP.SetDefaultSubobjectClass<UKriegerCharacterMovement>(ACharacter::CharacterMovementComponentName))
{
	Health = 100;
	MaxHealth = 100;
}


//////////////////////////////////////////////////////////////////////////
// Health

bool AKriegerCharacter::IsAlive() const
{
	return Health > 0;
}

float AKriegerCharacter::GetHealth() const
{
	return Health;
}

void AKriegerCharacter::SetHealth(float NewHealth)
{
	Health = NewHealth;
}

float AKriegerCharacter::GetMaxHealth() const
{
	return MaxHealth;//GetClass()->GetDefaultObject<AKriegerCharacter>()->Health;
}

void AKriegerCharacter::SetMaxHealth(float NewMaxHealth)
{
	MaxHealth = NewMaxHealth;
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
	DOREPLIFETIME(AKriegerCharacter, MaxHealth);
}

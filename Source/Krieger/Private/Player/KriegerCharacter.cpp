// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "Krieger.h"

AKriegerCharacter::AKriegerCharacter(const class FPostConstructInitializeProperties& PCIP) 
	: Super(PCIP.SetDefaultSubobjectClass<UKriegerCharacterMovement>(ACharacter::CharacterMovementComponentName))
{
	CapsuleComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CapsuleComponent->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
	CapsuleComponent->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	Health = 100;
	MaxHealth = 100;
}

void AKriegerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Role == ROLE_Authority)
	{
		Health = GetMaxHealth();
		SpawnDefaultInventory();
	}

	// Create material instance for setting team colors
	for (int32 iMat = 0; iMat < Mesh->GetNumMaterials(); iMat++)
	{
		MeshMIDs.Add(Mesh->CreateAndSetMaterialInstanceDynamic(iMat));
	}

	// Play respawn effects
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (RespawnFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(this, RespawnFX, GetActorLocation(), GetActorRotation());
		}

		if (RespawnSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, RespawnSound, GetActorLocation());
		}
	}
}

void AKriegerCharacter::Destroyed()
{
	Super::Destroyed();

	DestroyInventory();
}

void AKriegerCharacter::PossessedBy(class AController* InController)
{
	Super::PossessedBy(InController);

	// [server] as soon as PlayerState is assigned, set team colors of this pawn for local player
	UpdateTeamColorsAllMIDs();
}

void AKriegerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// [client] as soon as PlayerState is assigned, set team colors of this pawn for local player
	if (PlayerState != NULL)
	{
		UpdateTeamColorsAllMIDs();
	}
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
// Inventory

void AKriegerCharacter::SpawnDefaultInventory()
{
	if (Role < ROLE_Authority)
	{
		return;
	}

	// @TODO Spawn modules and other inventory items
}

void AKriegerCharacter::DestroyInventory()
{
	if (Role < ROLE_Authority)
	{
		return;
	}

	// @TODO Remove all modules and weapons from inventory and destroy them
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage

void AKriegerCharacter::StartFire()
{
	if (!bWantsToFire)
	{
		bWantsToFire = true;
		
		StartWeaponFire();
	}
}

void AKriegerCharacter::StopFire()
{
	if (bWantsToFire)
	{
		bWantsToFire = false;
		
		StopWeaponFire();
	}
}

bool AKriegerCharacter::CanFire() const
{
	return IsAlive();
}

bool AKriegerCharacter::CanReload() const
{
	return true;
}

bool AKriegerCharacter::IsFiring() const
{
	return bWantsToFire;
};


//////////////////////////////////////////////////////////////////////////
// Meshes

void AKriegerCharacter::UpdateTeamColors(UMaterialInstanceDynamic* UseMID)
{
	if (UseMID)
	{
		AKriegerPlayerState* MyPlayerState = Cast<AKriegerPlayerState>(PlayerState);
		if (MyPlayerState != NULL)
		{
			float MaterialParam = (float)MyPlayerState->GetTeamNum();
			UseMID->SetScalarParameterValue(TEXT("Team Color Index"), MaterialParam);
		}
	}
}

void AKriegerCharacter::UpdateTeamColorsAllMIDs()
{
	for (int32 i = 0; i < MeshMIDs.Num(); ++i)
	{
		UpdateTeamColors(MeshMIDs[i]);
	}
}


//////////////////////////////////////////////////////////////////////////
// Replication

void AKriegerCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AKriegerCharacter, Health);
	DOREPLIFETIME(AKriegerCharacter, MaxHealth);
}

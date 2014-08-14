// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "Krieger.h"

AKriegerMech::AKriegerMech(const class FPostConstructInitializeProperties& PCIP) 
	: Super(PCIP)
{
	Health = 100;
	MaxHealth = 100;
}

//////////////////////////////////////////////////////////////////////////
// Inventory

void AKriegerMech::SpawnDefaultInventory()
{
	Super::SpawnDefaultInventory();

	if (Role < ROLE_Authority)
	{
		return;
	}

	// Weapons can collide mech on spawn, it's okay
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	// Spawn and equip default weapons
	AKriegerWeapon* NewWeapon = GetWorld()->SpawnActor<AKriegerWeapon>(DefaultWeaponRight, SpawnInfo);
	AddWeapon(NewWeapon);
	EquipWeapon(&WeaponRight, NewWeapon);
}

void AKriegerMech::DestroyInventory()
{
	Super::DestroyInventory();

	if (Role < ROLE_Authority)
	{
		return;
	}

	// Remove all weapons from inventory and destroy them
	for (int32 i = WeaponInventory.Num() - 1; i >= 0; i--)
	{
		AKriegerWeapon* Weapon = WeaponInventory[i];
		if (Weapon)
		{
			RemoveWeapon(Weapon);
			Weapon->Destroy();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// Weapons (Inventory)

void AKriegerMech::AddWeapon(AKriegerWeapon* Weapon)
{
	if (Weapon && Role == ROLE_Authority)
	{
		Weapon->OnEnterInventory(this);
		WeaponInventory.AddUnique(Weapon);
	}
}

void AKriegerMech::RemoveWeapon(AKriegerWeapon* Weapon)
{
	if (Weapon && Role == ROLE_Authority)
	{
		Weapon->OnLeaveInventory();
		WeaponInventory.RemoveSingle(Weapon);
	}
}

void AKriegerMech::EquipWeapon(AKriegerWeapon** WeaponSlot, AKriegerWeapon* Weapon)
{
	if (WeaponSlot && Weapon)
	{
		if (Role == ROLE_Authority)
		{
			SetCurrentWeapon(WeaponSlot, Weapon);
		}
		else
		{
			// @TODO RPC from client to server for weapon equip
		}
	}
}

void AKriegerMech::SetCurrentWeapon(AKriegerWeapon** WeaponSlot, AKriegerWeapon* NewWeapon, AKriegerWeapon* LastWeapon)
{
	AKriegerWeapon* LocalLastWeapon = NULL;

	if (LastWeapon != NULL)
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != *WeaponSlot)
	{
		LocalLastWeapon = *WeaponSlot;
	}

	// Unequip previous
	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnEquip();
	}

	*WeaponSlot = NewWeapon;

	// Equip new one
	if (NewWeapon)
	{
		// Make sure weapon's MyPawn is pointing back to us. During replication, 
		// we can't guarantee APawn::CurrentWeapon will rep after AWeapon::MyPawn!
		NewWeapon->SetOwningPawn(this);
		NewWeapon->OnEquip();
	}
}


//////////////////////////////////////////////////////////////////////////
// Replication

void AKriegerMech::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Only to local owner: weapon change requests are locally instigated, other clients don't need it
	DOREPLIFETIME_CONDITION(AKriegerMech, WeaponInventory, COND_OwnerOnly);

	// Everyone
	DOREPLIFETIME(AKriegerMech, WeaponRight);
	DOREPLIFETIME(AKriegerMech, WeaponLeft);
	DOREPLIFETIME(AKriegerMech, WeaponBody);
	DOREPLIFETIME(AKriegerMech, WeaponBack);
}

void AKriegerMech::OnRep_WeaponRight(AKriegerWeapon* LastWeapon)
{
	SetCurrentWeapon(&WeaponRight, WeaponRight, LastWeapon);
}

void AKriegerMech::OnRep_WeaponLeft(AKriegerWeapon* LastWeapon)
{
	SetCurrentWeapon(&WeaponLeft, WeaponLeft, LastWeapon);
}

void AKriegerMech::OnRep_WeaponBody(AKriegerWeapon* LastWeapon)
{
	SetCurrentWeapon(&WeaponBody, WeaponBody, LastWeapon);
}

void AKriegerMech::OnRep_WeaponBack(AKriegerWeapon* LastWeapon)
{
	SetCurrentWeapon(&WeaponBack, WeaponBack, LastWeapon);
}

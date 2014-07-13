// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "Krieger.h"

AKriegerWeapon::AKriegerWeapon(const class FPostConstructInitializeProperties& PCIP) : Super(PCIP)
{
	Mesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("WeaponMesh"));
	Mesh->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh->bChartDistanceFactor = true;
	Mesh->CastShadow = true;
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Mesh->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
	RootComponent = Mesh;

	bLoopedMuzzleFX = false;
	bLoopedFireAnim = false;
	bPlayingFireAnim = false;
	bIsEquipped = false;
	bWantsToFire = false;
	bPendingReload = false;
	bPendingEquip = false;
	CurrentState = EWeaponState::Idle;

	CurrentAmmo = 0;
	CurrentAmmoInClip = 0;
	BurstCounter = 0;
	LastFireTime = 0.0f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bReplicateInstigator = true;
	bNetUseOwnerRelevancy = true;

	bInfiniteAmmo = false;
	bInfiniteClip = false;
	MaxAmmo = 100;
	AmmoPerClip = 20;
	InitialClips = 4;
	TimeBetweenShots = 0.2f;
	NoAnimReloadDuration = 1.0f;
}

void AKriegerWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (InitialClips > 0)
	{
		CurrentAmmoInClip = AmmoPerClip;
		CurrentAmmo = AmmoPerClip * InitialClips;
	}
}

void AKriegerWeapon::Destroyed()
{
	Super::Destroyed();

	StopSimulatingWeaponFire();
}


//////////////////////////////////////////////////////////////////////////
// Inventory

void AKriegerWeapon::OnEquip()
{
	bPendingEquip = true;
	DetermineWeaponState();

	float Duration = PlayWeaponAnimation(EquipAnim);
	if (Duration <= 0.0f)
	{
		// failsafe
		Duration = 0.5f;
	}

	EquipStartedTime = GetWorld()->GetTimeSeconds();
	EquipDuration = Duration;

	GetWorldTimerManager().SetTimer(this, &AKriegerWeapon::OnEquipFinished, Duration, false);

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		PlayWeaponSound(EquipSound);
	}
}

void AKriegerWeapon::OnEquipFinished()
{
	bIsEquipped = true;
	bPendingEquip = false;

	if (MyPawn)
	{
		// try to reload empty clip
		if (MyPawn->IsLocallyControlled() && CurrentAmmoInClip <= 0 && CanReload())
		{
			StartReload();
		}
	}

	DetermineWeaponState();
}

void AKriegerWeapon::OnUnEquip()
{
	bIsEquipped = false;

	StopFire();

	if (bPendingReload)
	{
		StopWeaponAnimation(ReloadAnim);
		bPendingReload = false;

		GetWorldTimerManager().ClearTimer(this, &AKriegerWeapon::StopReload);
		GetWorldTimerManager().ClearTimer(this, &AKriegerWeapon::ReloadWeapon);
	}

	if (bPendingEquip)
	{
		StopWeaponAnimation(EquipAnim);
		bPendingEquip = false;

		GetWorldTimerManager().ClearTimer(this, &AKriegerWeapon::OnEquipFinished);
	}

	DetermineWeaponState();
}

void AKriegerWeapon::OnEnterInventory(AKriegerCharacter* NewOwner)
{
	SetOwningPawn(NewOwner);
}

void AKriegerWeapon::OnLeaveInventory()
{
	if (Role == ROLE_Authority)
	{
		SetOwningPawn(NULL);
	}

	if (IsAttachedToPawn())
	{
		OnUnEquip();
	}
}


//////////////////////////////////////////////////////////////////////////
// Input

void AKriegerWeapon::StartFire()
{
	if (Role < ROLE_Authority)
	{
		ServerStartFire();
	}

	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}

void AKriegerWeapon::StopFire()
{
	if (Role < ROLE_Authority)
	{
		ServerStopFire();
	}

	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}

void AKriegerWeapon::StartReload(bool bFromReplication)
{
	if (!bFromReplication && Role < ROLE_Authority)
	{
		ServerStartReload();
	}

	if (bFromReplication || CanReload())
	{
		bPendingReload = true;
		DetermineWeaponState();

		float AnimDuration = PlayWeaponAnimation(ReloadAnim);		
		if (AnimDuration <= 0.0f)
		{
			AnimDuration = NoAnimReloadDuration;
		}

		GetWorldTimerManager().SetTimer(this, &AKriegerWeapon::StopReload, AnimDuration, false);
		if (Role == ROLE_Authority)
		{
			GetWorldTimerManager().SetTimer(this, &AKriegerWeapon::ReloadWeapon, FMath::Max(0.1f, AnimDuration - 0.1f), false);
		}
		
		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			PlayWeaponSound(ReloadSound);
		}
	}
}

void AKriegerWeapon::StopReload()
{
	if (CurrentState == EWeaponState::Reloading)
	{
		bPendingReload = false;
		DetermineWeaponState();
		StopWeaponAnimation(ReloadAnim);
	}
}

bool AKriegerWeapon::ServerStartFire_Validate()
{
	return true;
}

void AKriegerWeapon::ServerStartFire_Implementation()
{
	StartFire();
}

bool AKriegerWeapon::ServerStopFire_Validate()
{
	return true;
}

void AKriegerWeapon::ServerStopFire_Implementation()
{
	StopFire();
}

bool AKriegerWeapon::ServerStartReload_Validate()
{
	return true;
}

void AKriegerWeapon::ServerStartReload_Implementation()
{
	StartReload();
}

bool AKriegerWeapon::ServerStopReload_Validate()
{
	return true;
}

void AKriegerWeapon::ServerStopReload_Implementation()
{
	StopReload();
}

void AKriegerWeapon::ClientStartReload_Implementation()
{
	StartReload();
}


//////////////////////////////////////////////////////////////////////////
// Control

bool AKriegerWeapon::CanFire() const
{
	bool bCanFire = MyPawn && MyPawn->CanFire();
	bool bStateOKToFire = ( ( CurrentState ==  EWeaponState::Idle ) || ( CurrentState == EWeaponState::Firing) );	
	return (( bCanFire == true ) && ( bStateOKToFire == true ) && ( bPendingReload == false ));
}

bool AKriegerWeapon::CanReload() const
{
	bool bCanReload = (!MyPawn || MyPawn->CanReload());
	bool bGotAmmo = ( CurrentAmmoInClip < AmmoPerClip) && (CurrentAmmo - CurrentAmmoInClip > 0 || HasInfiniteClip());
	bool bStateOKToReload = ( ( CurrentState ==  EWeaponState::Idle ) || ( CurrentState == EWeaponState::Firing) );
	return ( ( bCanReload == true ) && ( bGotAmmo == true ) && ( bStateOKToReload == true) );	
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage

void AKriegerWeapon::GiveAmmo(int32 AddAmount)
{
	const int32 MissingAmmo = FMath::Max(0, MaxAmmo - CurrentAmmo);
	AddAmount = FMath::Min(AddAmount, MissingAmmo);
	CurrentAmmo += AddAmount;

	// @TODO Check ammo for bot
	
	// Start reload if clip was empty
	if (GetCurrentAmmoInClip() <= 0 && CanReload())
	{
		// @TODO Check that current weapon is active

		// Reload
		ClientStartReload();
	}
}

void AKriegerWeapon::UseAmmo()
{
	if (!HasInfiniteAmmo())
	{
		CurrentAmmoInClip--;
	}

	if (!HasInfiniteAmmo() && !HasInfiniteClip())
	{
		CurrentAmmo--;
	}

	// @TODO Check ammo for bot
}

void AKriegerWeapon::HandleFiring()
{
	if ((CurrentAmmoInClip > 0 || HasInfiniteClip() || HasInfiniteAmmo()) && CanFire())
	{
		if (GetNetMode() != NM_DedicatedServer)
		{
			SimulateWeaponFire();
		}

		if (MyPawn && MyPawn->IsLocallyControlled())
		{
			FireWeapon();

			UseAmmo();
			
			// update firing FX on remote clients if function was called on server
			BurstCounter++;
		}
	}
	else if (CanReload())
	{
		StartReload();
	}
	else if (MyPawn && MyPawn->IsLocallyControlled())
	{
		if (GetCurrentAmmo() == 0 && !bRefiring)
		{
			PlayWeaponSound(OutOfAmmoSound);

			OutOfAmmo.Broadcast();
		}
		
		// stop weapon fire FX, but stay in Firing state
		if (BurstCounter > 0)
		{
			OnBurstFinished();
		}
	}

	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		// local client will notify server
		if (Role < ROLE_Authority)
		{
			ServerHandleFiring();
		}

		// reload after firing last round
		if (CurrentAmmoInClip <= 0 && CanReload())
		{
			StartReload();
		}

		// setup refire timer
		bRefiring = (CurrentState == EWeaponState::Firing && TimeBetweenShots > 0.0f);
		if (bRefiring)
		{
			GetWorldTimerManager().SetTimer(this, &AKriegerWeapon::HandleFiring, TimeBetweenShots, false);
		}
	}

	LastFireTime = GetWorld()->GetTimeSeconds();
}

bool AKriegerWeapon::ServerHandleFiring_Validate()
{
	return true;
}

void AKriegerWeapon::ServerHandleFiring_Implementation()
{
	const bool bShouldUpdateAmmo = (CurrentAmmoInClip > 0 && CanFire());

	HandleFiring();

	if (bShouldUpdateAmmo)
	{
		// update ammo
		UseAmmo();

		// update firing FX on remote clients
		BurstCounter++;
	}
}

void AKriegerWeapon::ReloadWeapon()
{
	int32 ClipDelta = FMath::Min(AmmoPerClip - CurrentAmmoInClip, CurrentAmmo - CurrentAmmoInClip);

	if (HasInfiniteClip())
	{
		ClipDelta = AmmoPerClip - CurrentAmmoInClip;
	}

	if (ClipDelta > 0)
	{
		CurrentAmmoInClip += ClipDelta;
	}

	if (HasInfiniteClip())
	{
		CurrentAmmo = FMath::Max(CurrentAmmoInClip, CurrentAmmo);
	}
}

void AKriegerWeapon::SetWeaponState(EWeaponState::Type NewState)
{
	const EWeaponState::Type PrevState = CurrentState;

	if (PrevState == EWeaponState::Firing && NewState != EWeaponState::Firing)
	{
		OnBurstFinished();
	}

	CurrentState = NewState;

	if (PrevState != EWeaponState::Firing && NewState == EWeaponState::Firing)
	{
		OnBurstStarted();
	}
}

void AKriegerWeapon::DetermineWeaponState()
{
	EWeaponState::Type NewState = EWeaponState::Idle;

	if (bIsEquipped)
	{
		if( bPendingReload  )
		{
			if( CanReload() == false )
			{
				NewState = CurrentState;
			}
			else
			{
				NewState = EWeaponState::Reloading;
			}
		}		
		else if ( (bPendingReload == false ) && ( bWantsToFire == true ) && ( CanFire() == true ))
		{
			NewState = EWeaponState::Firing;
		}
	}
	else if (bPendingEquip)
	{
		NewState = EWeaponState::Equipping;
	}

	SetWeaponState(NewState);
}

void AKriegerWeapon::OnBurstStarted()
{
	// start firing, can be delayed to satisfy TimeBetweenShots
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 && TimeBetweenShots > 0.0f && (LastFireTime + TimeBetweenShots) > GameTime)
	{
		GetWorldTimerManager().SetTimer(this, &AKriegerWeapon::HandleFiring, LastFireTime + TimeBetweenShots - GameTime, false);
	}
	else
	{
		HandleFiring();
	}
}

void AKriegerWeapon::OnBurstFinished()
{
	// stop firing FX on remote clients
	BurstCounter = 0;

	// stop firing FX locally, unless it's a dedicated server
	if (GetNetMode() != NM_DedicatedServer)
	{
		StopSimulatingWeaponFire();
	}
	
	GetWorldTimerManager().ClearTimer(this, &AKriegerWeapon::HandleFiring);
	bRefiring = false;
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage helpers

UAudioComponent* AKriegerWeapon::PlayWeaponSound(USoundCue* Sound)
{
	UAudioComponent* AC = NULL;
	if (Sound && MyPawn)
	{
		AC = UGameplayStatics::PlaySoundAttached(Sound, MyPawn->GetRootComponent());
	}

	return AC;
}

float AKriegerWeapon::PlayWeaponAnimation(UAnimMontage* Animation)
{
	float Duration = 0.0f;

	if (MyPawn && Animation)
	{
		Duration = MyPawn->PlayAnimMontage(Animation);
	}

	return Duration;
}

void AKriegerWeapon::StopWeaponAnimation(UAnimMontage* Animation)
{
	if (MyPawn && Animation)
	{
		MyPawn->StopAnimMontage(Animation);
	}
}

FVector AKriegerWeapon::GetMuzzleLocation() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketLocation(MuzzleAttachPoint);
}

FVector AKriegerWeapon::GetMuzzleDirection() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketRotation(MuzzleAttachPoint).Vector();
}

void AKriegerWeapon::SetOwningPawn(AKriegerCharacter* NewOwner)
{
	if (MyPawn != NewOwner)
	{
		Instigator = NewOwner;
		MyPawn = NewOwner;
		// net owner for RPC calls
		SetOwner(NewOwner);
	}	
}


//////////////////////////////////////////////////////////////////////////
// Replication & effects

void AKriegerWeapon::OnRep_MyPawn()
{
	if (MyPawn)
	{
		OnEnterInventory(MyPawn);
	}
	else
	{
		OnLeaveInventory();
	}
}

void AKriegerWeapon::OnRep_BurstCounter()
{
	if (BurstCounter > 0)
	{
		SimulateWeaponFire();
	}
	else
	{
		StopSimulatingWeaponFire();
	}
}

void AKriegerWeapon::OnRep_Reload()
{
	if (bPendingReload)
	{
		StartReload(true);
	}
	else
	{
		StopReload();
	}
}

void AKriegerWeapon::SimulateWeaponFire()
{
	if (Role == ROLE_Authority && CurrentState != EWeaponState::Firing)
	{
		return;
	}

	if (MuzzleFX)
	{
		USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
		if (!bLoopedMuzzleFX || MuzzlePSC == NULL)
		{
			MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, UseWeaponMesh, MuzzleAttachPoint);
		}
	}

	if (!bLoopedFireAnim || !bPlayingFireAnim)
	{
		PlayWeaponAnimation(FireAnim);
		bPlayingFireAnim = true;
	}

	if (bLoopedFireSound)
	{
		if (FireAC == NULL)
		{
			FireAC = PlayWeaponSound(FireLoopSound);
		}
	}
	else
	{
		PlayWeaponSound(FireSound);
	}

	AKriegerPlayerController* PC = (MyPawn != NULL) ? Cast<AKriegerPlayerController>(MyPawn->Controller) : NULL;
	if (PC != NULL && PC->IsLocalController())
	{
		if (FireCameraShake != NULL)
		{
			PC->ClientPlayCameraShake(FireCameraShake, 1);
		}
		if (FireForceFeedback != NULL)
		{
			PC->ClientPlayForceFeedback(FireForceFeedback, false, "Weapon");
		}
	}
}

void AKriegerWeapon::StopSimulatingWeaponFire()
{
	if (bLoopedMuzzleFX )
	{
		if( MuzzlePSC != NULL )
		{
			MuzzlePSC->DeactivateSystem();
			MuzzlePSC = NULL;
		}
		if( MuzzlePSCSecondary != NULL )
		{
			MuzzlePSCSecondary->DeactivateSystem();
			MuzzlePSCSecondary = NULL;
		}
	}

	if (bLoopedFireAnim && bPlayingFireAnim)
	{
		StopWeaponAnimation(FireAnim);
		bPlayingFireAnim = false;
	}

	if (FireAC)
	{
		FireAC->FadeOut(0.1f, 0.0f);
		FireAC = NULL;

		PlayWeaponSound(FireFinishSound);
	}
}

void AKriegerWeapon::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );

	DOREPLIFETIME( AKriegerWeapon, MyPawn );

	DOREPLIFETIME_CONDITION( AKriegerWeapon, CurrentAmmo,		COND_OwnerOnly );
	DOREPLIFETIME_CONDITION( AKriegerWeapon, CurrentAmmoInClip, COND_OwnerOnly );

	DOREPLIFETIME_CONDITION( AKriegerWeapon, BurstCounter,		COND_SkipOwner );
	DOREPLIFETIME_CONDITION( AKriegerWeapon, bPendingReload,	COND_SkipOwner );
}

USkeletalMeshComponent* AKriegerWeapon::GetWeaponMesh() const
{
	return Mesh;
}

class AKriegerCharacter* AKriegerWeapon::GetPawnOwner() const
{
	return MyPawn;
}

bool AKriegerWeapon::IsEquipped() const
{
	return bIsEquipped;
}

bool AKriegerWeapon::IsAttachedToPawn() const
{
	return bIsEquipped || bPendingEquip;
}

EWeaponState::Type AKriegerWeapon::GetCurrentState() const
{
	return CurrentState;
}

int32 AKriegerWeapon::GetCurrentAmmo() const
{
	return CurrentAmmo;
}

int32 AKriegerWeapon::GetCurrentAmmoInClip() const
{
	return CurrentAmmoInClip;
}

int32 AKriegerWeapon::GetAmmoPerClip() const
{
	return AmmoPerClip;
}

int32 AKriegerWeapon::GetMaxAmmo() const
{
	return MaxAmmo;
}

bool AKriegerWeapon::HasInfiniteAmmo() const
{
	//const AKriegerPlayerController* MyPC = (MyPawn != NULL) ? Cast<const AKriegerPlayerController>(MyPawn->Controller) : NULL;
	return bInfiniteAmmo;// || (MyPC && MyPC->HasInfiniteAmmo());
}

bool AKriegerWeapon::HasInfiniteClip() const
{
	//const AKriegerPlayerController* MyPC = (MyPawn != NULL) ? Cast<const AKriegerPlayerController>(MyPawn->Controller) : NULL;
	return bInfiniteClip;// || (MyPC && MyPC->HasInfiniteClip());
}

float AKriegerWeapon::GetEquipStartedTime() const
{
	return EquipStartedTime;
}

float AKriegerWeapon::GetEquipDuration() const
{
	return EquipDuration;
}

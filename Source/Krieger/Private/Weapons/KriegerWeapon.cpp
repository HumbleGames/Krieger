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

	// Instant hit weapon config
	CurrentFiringSpread = 0.0f;

	// Barrels
	CurrentBarrel = 0;
}

void AKriegerWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (InitialClips > 0)
	{
		CurrentAmmoInClip = AmmoPerClip;
		CurrentAmmo = AmmoPerClip * InitialClips;
	}

	DetachMeshFromPawn();
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
	AttachMeshToPawn();

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
	AttachMeshToPawn();

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
	DetachMeshFromPawn();
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

void AKriegerWeapon::AttachMeshToPawn()
{
	if (MyPawn)
	{
		// Be sure we removed prevous one result
		DetachMeshFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
		FName AttachPoint = MyPawn->GetWeaponAttachPoint(this);

		USkeletalMeshComponent* PawnMesh = MyPawn->GetPawnMesh();
		Mesh->AttachTo(PawnMesh, AttachPoint, EAttachLocation::SnapToTarget);
		Mesh->SetHiddenInGame(false);
	}
}

void AKriegerWeapon::DetachMeshFromPawn()
{
	Mesh->DetachFromParent();
	Mesh->SetHiddenInGame(true);
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
// Weapon fire

void AKriegerWeapon::FireWeapon()
{
	switch (WeaponType)
	{
	case EWeaponType::InstantHit:
		FireWeapon_Instant();
		break;

	case EWeaponType::Projectile:
		FireWeapon_Projectile();
		break;
	}

	// Use next barrel
	CurrentBarrel++;
	if (CurrentBarrel >= WeaponBarrels.Num())
	{
		CurrentBarrel = 0;
	}
}

void AKriegerWeapon::FireWeapon_Instant()
{
	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	const float CurrentSpread = GetCurrentSpread();
	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);

	const FVector AimDir = GetAdjustedAim();
	const FVector StartTrace = GetDamageStartLocation();
	const FVector ShootDir = WeaponRandomStream.VRandCone(AimDir, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDir * InstantConfig.WeaponRange;

	const FHitResult Impact = WeaponTrace(StartTrace, EndTrace);
	ProcessInstantHit(Impact, StartTrace, ShootDir, RandomSeed, CurrentSpread);

	CurrentFiringSpread = FMath::Min(InstantConfig.FiringSpreadMax, CurrentFiringSpread + InstantConfig.FiringSpreadIncrement);
}

void AKriegerWeapon::FireWeapon_Projectile()
{
	FVector ShootDir = GetAdjustedAim();
	FVector Origin = GetMuzzleLocation();

	// trace from origin to check what's under crosshair
	const float ProjectileAdjustRange = 10000.0f;
	const FVector StartTrace = GetDamageStartLocation();
	const FVector EndTrace = StartTrace + ShootDir * ProjectileAdjustRange;
	FHitResult Impact = WeaponTrace(StartTrace, EndTrace);

	// and adjust directions to hit that actor
	if (Impact.bBlockingHit)
	{
		const FVector AdjustedDir = (Impact.ImpactPoint - Origin).SafeNormal();
		bool bWeaponPenetration = false;

		const float DirectionDot = FVector::DotProduct(AdjustedDir, ShootDir);
		if (DirectionDot < 0.0f)
		{
			// shooting backwards = weapon is penetrating
			bWeaponPenetration = true;
		}
		else if (DirectionDot < 0.5f)
		{
			// check for weapon penetration if angle difference is big enough
			// raycast along weapon mesh to check if there's blocking hit

			FVector MuzzleStartTrace = Origin - GetMuzzleDirection() * 150.0f;
			FVector MuzzleEndTrace = Origin;
			FHitResult MuzzleImpact = WeaponTrace(MuzzleStartTrace, MuzzleEndTrace);

			if (MuzzleImpact.bBlockingHit)
			{
				bWeaponPenetration = true;
			}
		}

		if (bWeaponPenetration)
		{
			// spawn at crosshair position
			Origin = Impact.ImpactPoint - ShootDir * 10.0f;
		}
		else
		{
			// adjust direction to hit
			ShootDir = AdjustedDir;
		}
	}

	ServerFireProjectile(Origin, ShootDir);
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

	// Weapon type related stuff
	switch (WeaponType)
	{
	case EWeaponType::InstantHit:
		CurrentFiringSpread = 0.0f;
		break;

	default:
		break;
	}
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

FVector AKriegerWeapon::GetTargetPoint() const
{
	if (MyPawn)
	{
		return MyPawn->GetTargetPoint();
	}

	return FVector::ZeroVector;
}

FVector AKriegerWeapon::GetAdjustedAim() const
{
	FVector FinalAim = GetTargetPoint() - GetDamageStartLocation();
	FinalAim.Normalize();

	return FinalAim;
}

FVector AKriegerWeapon::GetDamageStartLocation() const
{
	return GetMuzzleLocation();
}

FVector AKriegerWeapon::GetMuzzleLocation() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketLocation(GetCurrentBarrelSocketName());
}

FVector AKriegerWeapon::GetMuzzleDirection() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketRotation(GetCurrentBarrelSocketName()).Vector();
}

FName AKriegerWeapon::GetCurrentBarrelSocketName() const
{
	if (CurrentBarrel < 0 || CurrentBarrel >= WeaponBarrels.Num())
	{
		return TEXT("");
	}

	return WeaponBarrels[CurrentBarrel].MuzzleAttachPoint;
}

FHitResult AKriegerWeapon::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace) const
{
	static FName WeaponFireTag = FName(TEXT("WeaponTrace"));

	// Perform trace to retrieve hit info
	FCollisionQueryParams TraceParams(WeaponFireTag, true, Instigator);
	TraceParams.bTraceAsyncScene = true;
	TraceParams.bReturnPhysicalMaterial = true;

	FHitResult Hit(ForceInit);
	GetWorld()->LineTraceSingle(Hit, StartTrace, EndTrace, COLLISION_WEAPON, TraceParams);

	return Hit;
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

		int32 NumBarrels = WeaponBarrels.Num();
		for (int32 i = 0; i < NumBarrels; i++)
		{
			if (!bSimultaneousMuzzleFX && i != CurrentBarrel)
			{
				if (WeaponBarrels[i].MuzzlePSC != NULL)
				{
					WeaponBarrels[i].MuzzlePSC->DeactivateSystem();
					WeaponBarrels[i].MuzzlePSC = NULL;
				}

				continue;
			}

			if (!bLoopedMuzzleFX || WeaponBarrels[i].MuzzlePSC == NULL)
			{
				WeaponBarrels[i].MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, UseWeaponMesh, WeaponBarrels[i].MuzzleAttachPoint);
			}
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
		int32 NumBarrels = WeaponBarrels.Num();
		for (int32 i = 0; i < NumBarrels; i++)
		{
			if (WeaponBarrels[i].MuzzlePSC != NULL)
			{
				WeaponBarrels[i].MuzzlePSC->DeactivateSystem();
				WeaponBarrels[i].MuzzlePSC = NULL;
			}
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

	// Instant hit weapon
	DOREPLIFETIME_CONDITION(AKriegerWeapon, HitNotify, COND_SkipOwner);
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



//////////////////////////////////////////////////////////////////////////
// INSTANT HIT WEAPON
//////////////////////////////////////////////////////////////////////////

bool AKriegerWeapon::ServerNotifyHit_Validate(const FHitResult Impact, FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread)
{
	return true;
}

void AKriegerWeapon::ServerNotifyHit_Implementation(const FHitResult Impact, FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread)
{
	const float WeaponAngleDot = FMath::Abs(FMath::Sin(ReticleSpread * PI / 180.f));

	// if we have an instigator, calculate dot between the view and the shot
	if (Instigator && (Impact.GetActor() || Impact.bBlockingHit))
	{
		const FVector Origin = GetMuzzleLocation();
		const FVector ViewDir = (Impact.Location - Origin).SafeNormal();

		// is the angle between the hit and the view within allowed limits (limit + weapon max angle)
		const float ViewDotHitDir = FVector::DotProduct(Instigator->GetViewRotation().Vector(), ViewDir);
		if (ViewDotHitDir > InstantConfig.AllowedViewDotHitDir - WeaponAngleDot)
		{
			if (CurrentState != EWeaponState::Idle)
			{
				if (Impact.GetActor() == NULL)
				{
					if (Impact.bBlockingHit)
					{
						ProcessInstantHit_Confirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
					}
				}
				// assume it told the truth about static things because the don't move and the hit 
				// usually doesn't have significant gameplay implications
				else if (Impact.GetActor()->IsRootComponentStatic() || Impact.GetActor()->IsRootComponentStationary())
				{
					ProcessInstantHit_Confirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
				}
				else
				{
					// Get the component bounding box
					const FBox HitBox = Impact.GetActor()->GetComponentsBoundingBox();

					// calculate the box extent, and increase by a leeway
					FVector BoxExtent = 0.5 * (HitBox.Max - HitBox.Min);
					BoxExtent *= InstantConfig.ClientSideHitLeeway;

					// avoid precision errors with really thin objects
					BoxExtent.X = FMath::Max(20.0f, BoxExtent.X);
					BoxExtent.Y = FMath::Max(20.0f, BoxExtent.Y);
					BoxExtent.Z = FMath::Max(20.0f, BoxExtent.Z);

					// Get the box center
					const FVector BoxCenter = (HitBox.Min + HitBox.Max) * 0.5;

					// if we are within client tolerance
					if (FMath::Abs(Impact.Location.Z - BoxCenter.Z) < BoxExtent.Z &&
						FMath::Abs(Impact.Location.X - BoxCenter.X) < BoxExtent.X &&
						FMath::Abs(Impact.Location.Y - BoxCenter.Y) < BoxExtent.Y)
					{
						ProcessInstantHit_Confirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
					}
					else
					{
						UE_LOG(LogWeapon, Log, TEXT("%s Rejected client side hit of %s (outside bounding box tolerance)"), *GetNameSafe(this), *GetNameSafe(Impact.GetActor()));
					}
				}
			}
		}
		else if (ViewDotHitDir <= InstantConfig.AllowedViewDotHitDir)
		{
			UE_LOG(LogWeapon, Log, TEXT("%s Rejected client side hit of %s (facing too far from the hit direction)"), *GetNameSafe(this), *GetNameSafe(Impact.GetActor()));
		}
		else
		{
			UE_LOG(LogWeapon, Log, TEXT("%s Rejected client side hit of %s"), *GetNameSafe(this), *GetNameSafe(Impact.GetActor()));
		}
	}
}

bool AKriegerWeapon::ServerNotifyMiss_Validate(FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread)
{
	return true;
}

void AKriegerWeapon::ServerNotifyMiss_Implementation(FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread)
{
	const FVector Origin = GetMuzzleLocation();

	// play FX on remote clients
	HitNotify.Origin = Origin;
	HitNotify.RandomSeed = RandomSeed;
	HitNotify.ReticleSpread = ReticleSpread;

	// play FX locally
	if (GetNetMode() != NM_DedicatedServer)
	{
		const FVector EndTrace = Origin + ShootDir * InstantConfig.WeaponRange;
		SpawnTrailEffect(EndTrace);
	}
}

void AKriegerWeapon::ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread)
{
	if (MyPawn && MyPawn->IsLocallyControlled() && GetNetMode() == NM_Client)
	{
		// if we're a client and we've hit something that is being controlled by the server
		if (Impact.GetActor() && Impact.GetActor()->GetRemoteRole() == ROLE_Authority)
		{
			// notify the server of the hit
			ServerNotifyHit(Impact, ShootDir, RandomSeed, ReticleSpread);
		}
		else if (Impact.GetActor() == NULL)
		{
			if (Impact.bBlockingHit)
			{
				// notify the server of the hit
				ServerNotifyHit(Impact, ShootDir, RandomSeed, ReticleSpread);
			}
			else
			{
				// notify server of the miss
				ServerNotifyMiss(ShootDir, RandomSeed, ReticleSpread);
			}
		}
	}

	// process a confirmed hit
	ProcessInstantHit_Confirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
}

void AKriegerWeapon::ProcessInstantHit_Confirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread)
{
	// handle damage
	if (ShouldDealDamage(Impact.GetActor()))
	{
		DealDamage(Impact, ShootDir);
	}

	// play FX on remote clients
	if (Role == ROLE_Authority)
	{
		HitNotify.Origin = Origin;
		HitNotify.RandomSeed = RandomSeed;
		HitNotify.ReticleSpread = ReticleSpread;
	}

	// play FX locally
	if (GetNetMode() != NM_DedicatedServer)
	{
		const FVector EndTrace = Origin + ShootDir * InstantConfig.WeaponRange;
		const FVector EndPoint = Impact.GetActor() ? Impact.ImpactPoint : EndTrace;

		SpawnTrailEffect(EndPoint);
		SpawnImpactEffects(Impact);
	}
}

bool AKriegerWeapon::ShouldDealDamage(AActor* TestActor) const
{
	// if we're an actor on the server, or the actor's role is authoritative, we should register damage
	if (TestActor)
	{
		if (GetNetMode() != NM_Client ||
			TestActor->Role == ROLE_Authority ||
			TestActor->bTearOff)
		{
			return true;
		}
	}

	return false;
}

void AKriegerWeapon::DealDamage(const FHitResult& Impact, const FVector& ShootDir)
{
	FPointDamageEvent PointDmg;
	PointDmg.DamageTypeClass = InstantConfig.DamageType;
	PointDmg.HitInfo = Impact;
	PointDmg.ShotDirection = ShootDir;
	PointDmg.Damage = InstantConfig.HitDamage;

	Impact.GetActor()->TakeDamage(PointDmg.Damage, PointDmg, MyPawn->Controller, this);
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage helpers

float AKriegerWeapon::GetCurrentSpread() const
{
	float FinalSpread = InstantConfig.WeaponSpread + CurrentFiringSpread;

	// @TODO Walk/run and Targeting modificator
	/*if (MyPawn && MyPawn->IsTargeting())
	{
		FinalSpread *= InstantConfig.TargetingSpreadMod;
	}*/

	return FinalSpread;
}


//////////////////////////////////////////////////////////////////////////
// Replication & effects

void AKriegerWeapon::OnRep_HitNotify()
{
	SimulateInstantHit(HitNotify.Origin, HitNotify.RandomSeed, HitNotify.ReticleSpread);
}

void AKriegerWeapon::SimulateInstantHit(const FVector& ShotOrigin, int32 RandomSeed, float ReticleSpread)
{
	FRandomStream WeaponRandomStream(RandomSeed);
	const float ConeHalfAngle = FMath::DegreesToRadians(ReticleSpread * 0.5f);

	const FVector StartTrace = ShotOrigin;
	const FVector AimDir = GetAdjustedAim();
	const FVector ShootDir = WeaponRandomStream.VRandCone(AimDir, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDir * InstantConfig.WeaponRange;

	FHitResult Impact = WeaponTrace(StartTrace, EndTrace);
	if (Impact.bBlockingHit)
	{
		SpawnImpactEffects(Impact);
		SpawnTrailEffect(Impact.ImpactPoint);
	}
	else
	{
		SpawnTrailEffect(EndTrace);
	}
}

void AKriegerWeapon::SpawnImpactEffects(const FHitResult& Impact)
{
	if (ImpactTemplate && Impact.bBlockingHit)
	{
		FHitResult UseImpact = Impact;

		// trace again to find component lost during replication
		if (!Impact.Component.IsValid())
		{
			const FVector StartTrace = Impact.ImpactPoint + Impact.ImpactNormal * 10.0f;
			const FVector EndTrace = Impact.ImpactPoint - Impact.ImpactNormal * 10.0f;
			FHitResult Hit = WeaponTrace(StartTrace, EndTrace);
			UseImpact = Hit;
		}

		AKriegerImpactEffect* EffectActor = GetWorld()->SpawnActorDeferred<AKriegerImpactEffect>(ImpactTemplate, Impact.ImpactPoint, Impact.ImpactNormal.Rotation());
		if (EffectActor)
		{
			EffectActor->SurfaceHit = UseImpact;
			UGameplayStatics::FinishSpawningActor(EffectActor, FTransform(Impact.ImpactNormal.Rotation(), Impact.ImpactPoint));
		}
	}
}

void AKriegerWeapon::SpawnTrailEffect(const FVector& EndPoint)
{
	if (TrailFX)
	{
		const FVector Origin = GetMuzzleLocation();

		UParticleSystemComponent* TrailPSC = UGameplayStatics::SpawnEmitterAtLocation(this, TrailFX, Origin);
		if (TrailPSC)
		{
			TrailPSC->SetVectorParameter(TrailTargetParam, EndPoint);
		}
	}
}



//////////////////////////////////////////////////////////////////////////
// PROJECTILE WEAPON

bool AKriegerWeapon::ServerFireProjectile_Validate(FVector Origin, FVector_NetQuantizeNormal ShootDir)
{
	return true;
}

void AKriegerWeapon::ServerFireProjectile_Implementation(FVector Origin, FVector_NetQuantizeNormal ShootDir)
{
	FTransform SpawnTM(ShootDir.Rotation(), Origin);
	AKriegerProjectile* Projectile = Cast<AKriegerProjectile>(UGameplayStatics::BeginSpawningActorFromClass(this, ProjectileConfig.ProjectileClass, SpawnTM));
	if (Projectile)
	{
		Projectile->Instigator = Instigator;
		Projectile->SetOwner(this);
		Projectile->InitVelocity(ShootDir);

		UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
	}
}

void AKriegerWeapon::ApplyWeaponConfig(FProjectileWeaponData& Data)
{
	Data = ProjectileConfig;
}

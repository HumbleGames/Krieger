// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "Krieger.h"

AKriegerProjectile::AKriegerProjectile(const class FObjectInitializer& PCIP) : Super(PCIP)
{
	CollisionComp = PCIP.CreateDefaultSubobject<USphereComponent>(this, TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->AlwaysLoadOnClient = true;
	CollisionComp->AlwaysLoadOnServer = true;
	CollisionComp->bTraceComplexOnMove = true;
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(COLLISION_PROJECTILE);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_Destructible, ECR_Block);
	RootComponent = CollisionComp;

	ParticleComp = PCIP.CreateDefaultSubobject<UParticleSystemComponent>(this, TEXT("ParticleComp"));
	ParticleComp->bAutoActivate = true;
	ParticleComp->bAutoDestroy = false;
	ParticleComp->AttachParent = RootComponent;

	MovementComp = PCIP.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileComp"));
	MovementComp->UpdatedComponent = CollisionComp;
	MovementComp->InitialSpeed = 2000.0f;
	MovementComp->MaxSpeed = 2000.0f;
	MovementComp->bRotationFollowsVelocity = true;
	MovementComp->ProjectileGravityScale = 0.f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bReplicateMovement = true;
}

void AKriegerProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	MovementComp->OnProjectileStop.AddDynamic(this, &AKriegerProjectile::OnImpact);
	CollisionComp->MoveIgnoreActors.Add(Instigator);

	// override config from weapon
	AKriegerWeapon* OwnerWeapon = Cast<AKriegerWeapon>(GetOwner());
	if (OwnerWeapon)
	{
		OwnerWeapon->ApplyWeaponConfig(WeaponConfig);
	}
	
	MyController = GetInstigatorController();
}

void AKriegerProjectile::InitVelocity(FVector ShootDirection)
{
	if (MovementComp)
	{
		// @WTF Why we should remove it? Is velocity relative?
		//MovementComp->Velocity = ShootDirection * MovementComp->InitialSpeed;
	}
}

void AKriegerProjectile::SetWeaponConfig(const FProjectileWeaponData& Data)
{
	WeaponConfig = Data;

	SetLifeSpan(WeaponConfig.ProjectileLife);
}

void AKriegerProjectile::OnImpact(const FHitResult& HitResult)
{
	if (Role == ROLE_Authority && !bExploded)
	{
		Explode(HitResult);
		DisableAndDestroy();
	}
}

void AKriegerProjectile::Explode(const FHitResult& Impact)
{
	if (ParticleComp)
	{
		ParticleComp->Deactivate();
	}

	// Effects and damage origin shouldn't be placed inside mesh at impact point
	const FVector NudgedImpactLocation = Impact.ImpactPoint + Impact.ImpactNormal * 10.0f;

	if (WeaponConfig.ExplosionDamage > 0 && WeaponConfig.ExplosionRadius > 0 && WeaponConfig.DamageType)
	{
		UGameplayStatics::ApplyRadialDamage(this, WeaponConfig.ExplosionDamage, NudgedImpactLocation, WeaponConfig.ExplosionRadius, WeaponConfig.DamageType, TArray<AActor*>(), this, MyController.Get());
	}

	if (ExplosionTemplate)
	{
		const FRotator SpawnRotation = Impact.ImpactNormal.Rotation();

		AKriegerExplosionEffect* EffectActor = GetWorld()->SpawnActorDeferred<AKriegerExplosionEffect>(ExplosionTemplate, FTransform(SpawnRotation, NudgedImpactLocation));
		if (EffectActor)
		{
			EffectActor->SurfaceHit = Impact;
			UGameplayStatics::FinishSpawningActor(EffectActor, FTransform(SpawnRotation, NudgedImpactLocation));
		}
	}

	bExploded = true;
}

void AKriegerProjectile::DisableAndDestroy()
{
	UAudioComponent* ProjAudioComp = FindComponentByClass<UAudioComponent>();
	if (ProjAudioComp && ProjAudioComp->IsPlaying())
	{
		ProjAudioComp->FadeOut(0.1f, 0.f);
	}

	MovementComp->StopMovementImmediately();

	// Give clients some time to show explosion
	SetLifeSpan( 2.0f );
}

void AKriegerProjectile::OnRep_Exploded()
{
	FVector ProjDirection = GetActorRotation().Vector();

	const FVector StartTrace = GetActorLocation() - ProjDirection * 200;
	const FVector EndTrace = GetActorLocation() + ProjDirection * 150;
	FHitResult Impact;
	
	if (!GetWorld()->LineTraceSingleByChannel(Impact, StartTrace, EndTrace, COLLISION_PROJECTILE, FCollisionQueryParams(TEXT("ProjClient"), true, Instigator)))
	{
		// Failsafe
		Impact.ImpactPoint = GetActorLocation();
		Impact.ImpactNormal = -ProjDirection;
	}

	Explode(Impact);
}

void AKriegerProjectile::PostNetReceiveVelocity(const FVector& NewVelocity)
{
	if (MovementComp)
	{
		MovementComp->Velocity = NewVelocity;
	}
}

void AKriegerProjectile::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );
	
	DOREPLIFETIME( AKriegerProjectile, bExploded );
}

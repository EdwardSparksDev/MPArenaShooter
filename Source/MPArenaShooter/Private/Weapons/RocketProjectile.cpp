// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/RocketProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystemInstanceController.h"
#include "Components/AudioComponent.h"
#include "Components/RocketMovementComponent.h"
#include "Weapons/WeaponBase.h"


ARocketProjectile::ARocketProjectile()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->MaxSpeed = 2000.f;
	RocketMovementComponent->InitialSpeed = RocketMovementComponent->MaxSpeed;
	RocketMovementComponent->ProjectileGravityScale = 0.f;
}


void ARocketProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		HitboxSphere->OnComponentHit.AddDynamic(this, &ARocketProjectile::OnHit);
	}

	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailSystem, RootComponent, FName(), GetActorLocation(),
			GetActorRotation(), EAttachLocation::KeepWorldPosition, false);
	}

	if (LoopSound && LoopSoundAttenuation)
	{
		LoopSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopSound, RootComponent, FName(), GetActorLocation(),
			EAttachLocation::KeepWorldPosition, false, 1.f, 1.f, 0.f, LoopSoundAttenuation, nullptr, false);
	}
}


void ARocketProjectile::Destroyed() {}


void ARocketProjectile::OnHit(UPrimitiveComponent *HitComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, FVector NormalImpulse,
	const FHitResult &Hit)
{
	if (HasAuthority())
	{
		TArray<AActor *> IgnoredActors{ this };
		bool bHit = UGameplayStatics::ApplyRadialDamageWithFalloff(this, Damage, 10.f, GetActorLocation(), 200.f, 500.f, 1.f,
			UDamageType::StaticClass(), IgnoredActors, FiringWeapon, FiringController);
	}

	if (RocketMesh)
	{
		RocketMesh->SetVisibility(false);
	}

	if (HitboxSphere)
	{
		HitboxSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (ImpactVFX)
	{
		// Return the TraceVFX to its Pool.
		if (TracerComponent)
		{
			TracerComponent->ReleaseToPool();
		}

		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactVFX, GetActorTransform(), true, EPSCPoolMethod::AutoRelease);
	}

	// Play the ImpactSound at location.
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
	{
		TrailSystemComponent->GetSystemInstanceController()->Deactivate();
	}

	if (LoopSoundComponent && LoopSoundComponent->IsPlaying())
	{
		LoopSoundComponent->Stop();
	}

	SetLifeSpan(DestroyDelay);
}
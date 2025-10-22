// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Interfaces/DamageableInterface.h"
#include "Components/CombatComponent.h"
#include "Misc/ConstantsDefinitions.h"
#include "GameFramework/Character.h"
#include "Weapons/WeaponBase.h"


AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Set up projectile collision.
	HitboxSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(HitboxSphere);
	HitboxSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	HitboxSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HitboxSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	HitboxSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	HitboxSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	HitboxSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_SkeletalMesh, ECollisionResponse::ECR_Block);

	SetLifeSpan(10.f);
}


void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	// Only register hit events on the server.
	if (HasAuthority())
	{
		HitboxSphere->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
	}

	if (TracerVFX)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(TracerVFX, HitboxSphere, FName(), GetActorLocation(), GetActorRotation(),
			EAttachLocation::KeepWorldPosition, true, EPSCPoolMethod::ManualRelease);
	}
}


void AProjectileBase::Destroyed()
{
	Super::Destroyed();

	// Spawn the ImpactVFX.
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
}


void AProjectileBase::OnHit(UPrimitiveComponent *HitComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, FVector NormalImpulse,
	const FHitResult &Hit)
{
	if (!IsValid(OtherActor))
	{
		return;
	}

	// Apply damage.
	if (FiringController)
	{
		UGameplayStatics::ApplyDamage(OtherActor, Damage, FiringController, FiringWeapon, UDamageType::StaticClass());
	}

	Destroy();
}


void AProjectileBase::Initialize(AController *NewFiringController, AWeaponBase *NewFiringWeapon)
{
	FiringController = NewFiringController;
	FiringWeapon = NewFiringWeapon;

	Damage = NewFiringWeapon->GetDamage();
}
// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/AmmoCasing.h"
#include "Kismet/GameplayStatics.h"


AAmmoCasing::AAmmoCasing()
{
	PrimaryActorTick.bCanEverTick = false;

	// Set up CasingMesh.
	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_PhysicsBody, ECollisionResponse::ECR_Ignore);
	CasingMesh->SetNotifyRigidBodyCollision(true);
	CasingMesh->SetSimulatePhysics(true);

	SetLifeSpan(3.f);
}


void AAmmoCasing::BeginPlay()
{
	Super::BeginPlay();

	CasingMesh->OnComponentHit.AddDynamic(this, &AAmmoCasing::OnHit);
}


void AAmmoCasing::OnHit(UPrimitiveComponent *HitComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, FVector NormalImpulse,
	const FHitResult &Hit)
{
	// Disable hit notifies after first hit.
	CasingMesh->SetNotifyRigidBodyCollision(false);

	if (CasingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CasingSound, GetActorLocation());
	}
}


void AAmmoCasing::Eject(float Force)
{
	CasingMesh->AddImpulse(GetActorForwardVector() * Force);
}
// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/HealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"


UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Set up DissolveEffectTimeline.
	DissolveEffectTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveEffectTimeline"));
}


void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, Health);
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize the component's owner.
	Character = Cast<ACharacter>(GetOwner());

	if (!Character)
	{
		return;
	}

	// Only receive damage on the server.
	if (Character->HasAuthority())
	{
		Character->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::OnAnyDamage);
	}

	Character->GetWorldTimerManager().SetTimerForNextTick(this, &UHealthComponent::PostBeginPlay);
}


void UHealthComponent::PostBeginPlay()
{
	OnHealthChanged.Broadcast(Health, MaxHealth);
}


void UHealthComponent::SetComponentEnabled(bool bEnable)
{
	bComponentEnabled = bEnable;
}


void UHealthComponent::OnAnyDamage(AActor *DamagedActor, float Damage, const UDamageType *DamageType, AController *InstigatorController,
	AActor *DamageCauser)
{
	if (!bComponentEnabled || Health <= 0)
	{
		return;
	}

	// Update Health.
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(Health, MaxHealth);

	if (Health == 0.f)
	{
		OnPlayerEliminated.Broadcast(InstigatorController, Character->Controller, DamageCauser);
		EliminatePlayer();
	}
	else
	{
		PlayHitReaction();
	}
}


void UHealthComponent::OnRep_Health()
{
	OnHealthChanged.Broadcast(Health, MaxHealth);

	if (Health == 0.f)
	{
		AController *Controller = Character ? Character->Controller : nullptr;
		OnPlayerEliminated.Broadcast(nullptr, Controller, nullptr);
		EliminatePlayer();
	}
	else
	{
		PlayHitReaction();
	}
}


void UHealthComponent::PlayHitReaction()
{
	if (IsValid(Character))
	{
		// Play the hit reaction montage.
		UAnimInstance *AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance && HitReactionMontage)
		{
			EMontagePlayReturnType MontageReturnType = EMontagePlayReturnType::MontageLength;
			float PlayRate = 1.f;
			float TimeToStartMontageAt = 0.f;
			bool bStopAllMontages = false;
			AnimInstance->Montage_Play(HitReactionMontage, PlayRate, MontageReturnType, TimeToStartMontageAt, bStopAllMontages);
		}
	}
}


void UHealthComponent::EliminatePlayer()
{
	if (!IsValid(Character))
	{
		return;
	}

	PlayEliminationMontage();

	// If enabled, create and set a dynamic material instance of the character's current material to play the death dissolve effect.
	if (DissolveEffectTimeline)
	{
		USkeletalMeshComponent *SkeletalMeshComp = Character->GetMesh();
		if (SkeletalMeshComp && DissolveEffectMaterialInstance && DissolveEffectCurve)
		{
			// Create and set dynamic material instace.
			DynamicDissolveEffectMaterialInstance = UMaterialInstanceDynamic::Create(DissolveEffectMaterialInstance, this);
			SkeletalMeshComp->SetMaterial(DissolveEffectMaterialIndex, DynamicDissolveEffectMaterialInstance);

			// Initialize dynamic material instance's parameters.
			DynamicDissolveEffectMaterialInstance->SetScalarParameterValue(DissolveEffectAlphaParameterName,
				DissolveEffectCurve->GetFloatValue(0));

			StartDissolveEffect();
		}
	}

	// Spawn death vfx.
	if (DeathVFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DeathVFX, Character->GetActorLocation() +
			Character->GetActorRotation().RotateVector(DeathVFXOffset), Character->GetActorRotation(), true, EPSCPoolMethod::AutoRelease);
	}

	//Spawn death sound.
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathSound, Character->GetActorLocation());
	}
}


void UHealthComponent::PlayEliminationMontage()
{
	UAnimInstance *AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && EliminationMontage)
	{
		AnimInstance->Montage_Play(EliminationMontage);
	}
}


void UHealthComponent::StartDissolveEffect()
{
	DissolveEffectTrack.BindDynamic(this, &UHealthComponent::UpdateDissolveEffect);
	if (DissolveEffectCurve)
	{
		DissolveEffectTimeline->AddInterpFloat(DissolveEffectCurve, DissolveEffectTrack);
		DissolveEffectTimeline->Play();
	}
}


void UHealthComponent::UpdateDissolveEffect(float DissolveAlpha)
{
	if (DynamicDissolveEffectMaterialInstance)
	{
		DynamicDissolveEffectMaterialInstance->SetScalarParameterValue(DissolveEffectAlphaParameterName,
			DissolveEffectCurve->GetFloatValue(DissolveAlpha));
	}
}
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "HealthComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, Health, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerEliminatedSignature, AController *, AttackerController, AController *,
	EliminatedController, AActor *, DamageCauser);


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MPARENASHOOTER_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerEliminatedSignature OnPlayerEliminated;


	UHealthComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;


	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	FORCEINLINE float GetHealth() const { return Health; }

	FORCEINLINE bool IsDead() const { return Health <= 0.f; }


	UFUNCTION(BlueprintCallable)
	void SetComponentEnabled(bool bEnable);


protected:

	bool bComponentEnabled = true;


	virtual void BeginPlay() override;


	virtual void PostBeginPlay();


private:

	ACharacter *Character = nullptr;


	UPROPERTY(EditAnywhere, Category = "Combat|Stats")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Health, Category = "Combat|Stats")
	float Health = 100.f;


	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	UAnimMontage *HitReactionMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	UAnimMontage *EliminationMontage;


	UPROPERTY(EditAnywhere, Category = "Combat|Materials")
	UMaterialInstance *DissolveEffectMaterialInstance;

	UPROPERTY(EditAnywhere, Category = "Combat|Materials", meta = (EditCondition = "DissolveEffectMaterialInstance != nullptr",
		EditConditionHides))
	int DissolveEffectMaterialIndex = 0;

	UPROPERTY(EditAnywhere, Category = "Combat|Materials", meta = (EditCondition = "DissolveEffectMaterialInstance != nullptr",
		EditConditionHides))
	FName DissolveEffectAlphaParameterName = FName("DissolveAlpha");

	UPROPERTY(EditAnywhere, Category = "Combat|Materials", meta = (EditCondition = "DissolveEffectMaterialInstance != nullptr",
		EditConditionHides))
	UCurveFloat *DissolveEffectCurve;


	UPROPERTY(EditAnywhere, Category = "Combat|Visual Effects")
	UParticleSystem *DeathVFX;

	UPROPERTY(EditAnywhere, Category = "Combat|Visual Effects", meta = (EditCondition = "DeathVFX != nullptr", EditConditionHides))
	FVector DeathVFXOffset;


	UPROPERTY(EditAnywhere, Category = "Combat|Audio")
	USoundBase *DeathSound;


	// Elimination.
	FTimerHandle EliminationTimerHandle;
	UPROPERTY()
	UTimelineComponent *DissolveEffectTimeline;
	FOnTimelineFloat DissolveEffectTrack;
	UPROPERTY()
	UMaterialInstanceDynamic *DynamicDissolveEffectMaterialInstance;


	UFUNCTION()
	void OnAnyDamage(AActor *DamagedActor, float Damage, const UDamageType *DamageType, AController *InstigatorController, AActor *DamageCauser);

	UFUNCTION()
	void OnRep_Health();

	void PlayHitReaction();

	void EliminatePlayer();

	void PlayEliminationMontage();

	void StartDissolveEffect();

	UFUNCTION()
	void UpdateDissolveEffect(float DissolveAlpha);
};
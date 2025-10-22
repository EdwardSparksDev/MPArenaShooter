// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

class USphereComponent;
class UParticleSystem;
class AWeaponBase;


UCLASS()
class MPARENASHOOTER_API AProjectileBase : public AActor
{
	GENERATED_BODY()

public:

	AProjectileBase();

	virtual void Destroyed() override;


	void Initialize(AController *NewFiringController, AWeaponBase *NewFiringWeapon);


protected:

	UPROPERTY(EditAnywhere, Category = "Combat|VFX")
	UParticleSystem *TracerVFX;

	UPROPERTY(EditAnywhere, Category = "Combat|VFX")
	UParticleSystem *ImpactVFX;


	UPROPERTY(EditAnywhere, Category = "Combat|Sound")
	USoundBase *ImpactSound;


	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USphereComponent *HitboxSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UParticleSystemComponent *TracerComponent;


	UPROPERTY()
	AController *FiringController;

	UPROPERTY()
	AWeaponBase *FiringWeapon;


	float Damage;


	virtual void BeginPlay() override;


	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent *HitComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, FVector NormalImpulse,
		const FHitResult &Hit);
};
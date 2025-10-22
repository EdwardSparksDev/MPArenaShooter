// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "HitscanWeapon.generated.h"

class UParticleSystem;


UCLASS()
class MPARENASHOOTER_API AHitscanWeapon : public AWeaponBase
{
	GENERATED_BODY()

public:

	virtual void PerformFire(const FVector &Target, float AimValue) override;


protected:

	UPROPERTY(EditAnywhere, Category = "Combat|VFX")
	UParticleSystem *ImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Combat|VFX")
	UParticleSystem *BeamParticles;


	void WeaponTraceHit(const FVector &TraceStart, const FVector &HitTarget, FHitResult &OutHit, float Spread);
};
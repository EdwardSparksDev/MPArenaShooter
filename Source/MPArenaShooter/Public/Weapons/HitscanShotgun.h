// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/HitscanWeapon.h"
#include "HitscanShotgun.generated.h"


UCLASS()
class MPARENASHOOTER_API AHitscanShotgun : public AHitscanWeapon
{
	GENERATED_BODY()

public:

	virtual void PerformFire(const FVector &Target, float AimValue) override;


private:

	UPROPERTY(EditAnywhere, Category = "Combat|Stats")
	uint32 PelletsPerShot = 10;


	TMap<AActor *, uint32> ActorsHit;
};

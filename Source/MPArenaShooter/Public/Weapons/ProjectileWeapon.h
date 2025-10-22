// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "ProjectileWeapon.generated.h"

class AProjectileBase;


UCLASS()
class MPARENASHOOTER_API AProjectileWeapon : public AWeaponBase
{
	GENERATED_BODY()

public:

	virtual void PerformFire(const FVector &Target, float AimValue) override;


private:

	UPROPERTY(EditAnywhere, Category = "Combat|Stats")
	TSubclassOf<AProjectileBase> ProjectileClass;
};

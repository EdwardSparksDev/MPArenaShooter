// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/ProjectileBase.h"
#include "BulletProjectile.generated.h"

class UProjectileMovementComponent;


UCLASS()
class MPARENASHOOTER_API ABulletProjectile : public AProjectileBase
{
	GENERATED_BODY()

public:

	ABulletProjectile();


protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UProjectileMovementComponent *ProjectileMovementComponent;
};

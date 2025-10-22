// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/ProjectileBase.h"
#include "RocketProjectile.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class URocketMovementComponent;


UCLASS()
class MPARENASHOOTER_API ARocketProjectile : public AProjectileBase
{
	GENERATED_BODY()

public:

	ARocketProjectile();

	virtual void Destroyed() override;


protected:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent *RocketMesh;

	UPROPERTY(VisibleAnywhere)
	URocketMovementComponent *RocketMovementComponent;


	virtual void BeginPlay() override;


	virtual void OnHit(UPrimitiveComponent *HitComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, FVector NormalImpulse,
		const FHitResult &Hit);

private:

	UPROPERTY(EditAnywhere, Category = "Combat|VFX")
	float DestroyDelay = 3.f;

	UPROPERTY(EditAnywhere, Category = "Combat|VFX")
	UNiagaraSystem *TrailSystem;


	UPROPERTY(EditAnywhere, Category = "Combat|Audio")
	USoundBase *LoopSound;

	UPROPERTY(EditAnywhere, Category = "Combat|Audio")
	USoundAttenuation *LoopSoundAttenuation;


	FTimerHandle DestroyTimerHandle;

	UPROPERTY()
	UNiagaraComponent *TrailSystemComponent;

	UPROPERTY()
	UAudioComponent *LoopSoundComponent;
};
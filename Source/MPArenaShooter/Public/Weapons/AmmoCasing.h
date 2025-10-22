// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AmmoCasing.generated.h"


UCLASS()
class MPARENASHOOTER_API AAmmoCasing : public AActor
{
	GENERATED_BODY()

public:

	AAmmoCasing();


	void Eject(float Force);


protected:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent *CasingMesh;


	virtual void BeginPlay() override;


private:

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Sound")
	USoundBase *CasingSound;


	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent *HitComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, FVector NormalImpulse,
		const FHitResult &Hit);
};

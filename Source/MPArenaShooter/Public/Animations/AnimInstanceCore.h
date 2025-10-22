// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Types/TurningInPlace.h"
#include "Types/CombatState.h"
#include "AnimInstanceCore.generated.h"

class ACharacterCore;
class UCombatComponent;
class AWeaponBase;
class UHealthComponent;


UCLASS()
class MPARENASHOOTER_API UAnimInstanceCore : public UAnimInstance
{
	GENERATED_BODY()

public:

	virtual void NativeInitializeAnimation() override;

	virtual void NativeBeginPlay() override;

	virtual void NativeUpdateAnimation(float DeltaTime) override;


private:

	UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
	ACharacterCore *Character;

	UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
	UCombatComponent *CombatComponent;

	UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
	UHealthComponent *HealthComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsDead;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float VerticalSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float HorizontalSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bHasWeaponEquipped;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsAiming;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float YawOffset;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float Lean;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float LeaningSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	FVector2D LeaningClamp;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float AOYaw;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float AOPitch;

	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	AWeaponBase *EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = "Animations", meta = (AllowPrivateAccess = "true"))
	FRotator RightHandRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Animations", meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	ETurningInPlace TurningInPlace;

	UPROPERTY(BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bIsLocallyControlled;

	UPROPERTY(BlueprintReadOnly, Category = "Animations", meta = (AllowPrivateAccess = "true"))
	float BaseWalkReferenceSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Animations", meta = (AllowPrivateAccess = "true"))
	float AdsWalkReferenceSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Animations", meta = (AllowPrivateAccess = "true"))
	float BaseWalkReferenceSpeedCrouched;

	UPROPERTY(BlueprintReadOnly, Category = "Animations", meta = (AllowPrivateAccess = "true"))
	float AdsWalkReferenceSpeedCrouched;

	UPROPERTY(BlueprintReadOnly, Category = "Animations", meta = (AllowPrivateAccess = "true"))
	ECombatState CombatState;


	FName LeftHandSocketName;
	FName RightHandBoneName;
	FTransform RightHandTransform;

	FRotator LastCharacterRotation;
	FRotator CharacterRotation;

	float RotateWeaponToTargetSpeed;
};
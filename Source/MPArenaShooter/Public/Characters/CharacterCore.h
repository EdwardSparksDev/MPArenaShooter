// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/DamageableInterface.h"
#include "CharacterCore.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UWidgetComponent;
class AWeaponBase;
class UCombatComponent;
class UHealthComponent;
struct FInputActionValue;


UCLASS()
class MPARENASHOOTER_API ACharacterCore : public ACharacter, public IDamageableInterface
{
	GENERATED_BODY()

public:

	ACharacterCore();

	friend class UCombatComponent;

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;


protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USpringArmComponent *SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent *FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent *OverheadWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCombatComponent *CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UHealthComponent *HealthComponent;


	virtual void BeginPlay() override;


	UFUNCTION()
	virtual void DisableCombat();


private:

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext *Controls;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction *MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction *LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction *JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction *InteractAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction *CrouchAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction *AimAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction *FiringAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction *ReloadAction;


	void Move(const FInputActionValue &Value);

	void Look(const FInputActionValue &Value);

	void StartJump(const FInputActionValue &Value);

	void StopJump(const FInputActionValue &Value);

	void Interact(const FInputActionValue &Value);

	void ToggleCrouch(const FInputActionValue &Value);

	void StartAiming(const FInputActionValue &Value);

	void StopAiming(const FInputActionValue &Value);

	void StartFiring(const FInputActionValue &Value);

	void StopFiring(const FInputActionValue &Value);

	void Reload(const FInputActionValue &Value);
};
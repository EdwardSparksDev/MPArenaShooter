// Fill out your copyright notice in the Description page of Project Settings.

#include "Animations/AnimInstanceCore.h"
#include "Characters/CharacterCore.h"
#include "Components/CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapons/WeaponBase.h"
#include "Components/HealthComponent.h"


void UAnimInstanceCore::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// Get required references.
	Character = Cast<ACharacterCore>(TryGetPawnOwner());
	if (!Character)
	{
		return;
	}

	// Initialize CombatComponent variables.
	CombatComponent = Character->FindComponentByClass<UCombatComponent>();
	if (CombatComponent)
	{
		LeftHandSocketName = CombatComponent->GetLeftHandSocketName();
		RightHandBoneName = CombatComponent->GetRightHandBoneName();
		RotateWeaponToTargetSpeed = CombatComponent->GetRotateWeaponToTargetSpeed();
		LeaningSpeed = CombatComponent->GetLeaningSpeed();
		LeaningClamp = CombatComponent->GetLeaningClamp();

		BaseWalkReferenceSpeed = CombatComponent->GetBaseWalkReferenceSpeed();
		AdsWalkReferenceSpeed = CombatComponent->GetAdsWalkReferenceSpeed();
		BaseWalkReferenceSpeedCrouched = CombatComponent->GetBaseWalkReferenceSpeedCrouched();
		AdsWalkReferenceSpeedCrouched = CombatComponent->GetAdsWalkReferenceSpeedCrouched();
	}

	// Initialize HealthComponent variables.
	HealthComponent = Character->FindComponentByClass<UHealthComponent>();
}


void UAnimInstanceCore::NativeBeginPlay()
{
	if (Character)
	{
		bIsLocallyControlled = Character->IsLocallyControlled();
	}
}


void UAnimInstanceCore::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (!Character || !CombatComponent || !HealthComponent)
	{
		return;
	}

	// Status.
	bIsDead = HealthComponent->IsDead();
	CombatState = CombatComponent->GetCombatState();

	// Base movement.
	FVector Velocity = Character->GetVelocity();
	VerticalSpeed = Velocity.Z;
	Velocity.Z = 0.f;
	HorizontalSpeed = Velocity.Size();

	UCharacterMovementComponent *MovementComponent = Character->GetCharacterMovement();
	bIsInAir = MovementComponent->IsFalling();
	bIsAccelerating = MovementComponent->GetCurrentAcceleration().Size() > 0.f;

	// Equipped or unequipped.
	bHasWeaponEquipped = CombatComponent->HasWeaponEquipped();
	EquippedWeapon = CombatComponent->GetEquippedWeapon();

	// Aiming, crouching and turning.
	bIsCrouched = Character->bIsCrouched;
	bIsAiming = CombatComponent->IsAiming();
	TurningInPlace = CombatComponent->GetTurningInPlace();

	// Strafing.
	FRotator AimRotator = CombatComponent->GetRepControlRotation();
	FRotator MovementRotator = UKismetMathLibrary::MakeRotFromX(Velocity);
	YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotator, AimRotator).Yaw;

	// Leaning.
	LastCharacterRotation = CharacterRotation;
	CharacterRotation = Character->GetActorRotation();
	float Target = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, LastCharacterRotation).Yaw / DeltaTime;
	float InterpLean = FMath::FInterpTo(Lean, Target, DeltaTime, LeaningSpeed);
	Lean = FMath::Clamp(InterpLean, LeaningClamp.X, LeaningClamp.Y);

	// Aim Offset.
	AOYaw = CombatComponent->GetAOYaw();
	AOPitch = CombatComponent->GetAOPitch();

	// Calculate hands placements and rotations.
	if (EquippedWeapon && EquippedWeapon->GetWeaponMesh() && Character->GetMesh())
	{
		// Fix right hand rotation to match the camera trace hit location.
		RightHandTransform = Character->GetMesh()->GetSocketTransform(RightHandBoneName, ERelativeTransformSpace::RTS_World);
		FRotator TargetRot = (RightHandTransform.GetLocation() - CombatComponent->GetCameraTraceHitTarget()).Rotation();
		RightHandRotation = FMath::RInterpTo(RightHandRotation, TargetRot, DeltaTime, RotateWeaponToTargetSpeed);

		// FABRIK IK.
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(LeftHandSocketName, ERelativeTransformSpace::RTS_World);

		FVector IKLocation;
		FRotator IKRotation;
		Character->GetMesh()->TransformToBoneSpace(RightHandBoneName, LeftHandTransform.GetLocation(), LeftHandTransform.GetRotation().Rotator(),
			IKLocation, IKRotation);

		LeftHandTransform.SetLocation(IKLocation);
		LeftHandTransform.SetRotation(FQuat(IKRotation));
	}
}
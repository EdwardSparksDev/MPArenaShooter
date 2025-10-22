// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/CombatComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/DamageableInterface.h"
#include "Weapons/WeaponBase.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerControllers/PlayerControllerCore.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/HealthComponent.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}


void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// This function is required to register replicated variables.

	DOREPLIFETIME_CONDITION(UCombatComponent, RepCameraLocation, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(UCombatComponent, RepControlRotation, COND_SimulatedOnly);

	DOREPLIFETIME_CONDITION(UCombatComponent, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME_CONDITION(UCombatComponent, bIsAiming, COND_SkipOwner);

	DOREPLIFETIME(UCombatComponent, CombatState);
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize the component's owner.
	Character = Cast<ACharacter>(GetOwner());

	if (!Character)
	{
		return;
	}

	Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BaseWalkSpeedCrouched;

	USkeletalMeshComponent *SkeletalMeshComp = Character->GetMesh();
	if (SkeletalMeshComp)
	{
		SkeletalMeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}

	// Initialize CameraTraceTargetDotThreshold for tracing the target cone in TraceCameraTargetPoint().
	float CameraTraceTargetRadiansThreshold = FMath::DegreesToRadians((CameraTraceConeDegrees / 2.f));
	CameraTraceTargetDotThreshold = FMath::Cos(CameraTraceTargetRadiansThreshold);
	CurrentCameraFOV = DefaultCameraFOV;

	Character->GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::PostBeginPlay);
}


void UCombatComponent::PostBeginPlay()
{
	// Bind to player elimination delegate from the HealthComponent.
	UHealthComponent *HealthComp = Character->FindComponentByClass<UHealthComponent>();
	if (HealthComp)
	{
		HealthComp->OnPlayerEliminated.AddDynamic(this, &ThisClass::OnPlayerEliminated);
	}

	if (Character->GetLocalRole() != ENetRole::ROLE_SimulatedProxy)
	{
		FollowCamera = Character->FindComponentByClass<UCameraComponent>();

		if (Character->HasAuthority())
		{
			CameraLocationReplicationMinDistanceSqr = FMath::Pow(CameraLocationReplicationMinDistance, 2.);
		}
	}

	if (Character->IsLocallyControlled())
	{
		PlayerController = Cast<APlayerController>(Character->Controller);
		if (PlayerController)
		{
			// Temporary fix for vertical gimbal locking causing weapon hand to twist.
			PlayerController->PlayerCameraManager->ViewPitchMax = MaxViewPitch;
		}

		CameraOcclusionSqrDistance = FMath::Pow(CameraOcclusionDistance, 2.f);
		if (FollowCamera)
		{
			DefaultCameraFOV = FollowCamera->FieldOfView;
		}
	}

	CurrentCameraFOV = DefaultCameraFOV;

	//Broadcast weapon and crosshair changes.
	OnEquippedWeaponChanged.Broadcast(this);
	OnCrosshairSpreadChanged.Broadcast(CrosshairSpread);
	FLinearColor CrosshairColor = bAimingAtTarget ? TargetCrosshairColor : DefaultCrosshairColor;
	OnCrosshairColorChanged.Broadcast(CrosshairColor);

	if (Character->HasAuthority())
	{
		SpawnStartingWeapon();
	}
	else if (bStartingWeaponSpawnFailed)
	{
		bStartingWeaponSpawnFailed = false;
		OnRep_EquippedWeapon();
	}
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsValid(Character))
	{
		return;
	}

	if (Character->HasAuthority())
	{
		ReplicateCameraLocation(DeltaTime);
	}

	UpdateAimOffset(DeltaTime);
	TraceCameraTargetPoint();
	UpdateAimValue(DeltaTime);

	if (Character->IsLocallyControlled())
	{
		UpdateCharacterOcclusion();
		//UpdateWeaponSpread(DeltaTime);
	}
}


void UCombatComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	if (bShouldAutoDestroyEquippedWeapon && IsValid(EquippedWeapon))
	{
		EquippedWeapon->Destroy();
	}
}


void UCombatComponent::SetComponentEnabled(bool bEnable)
{
	bComponentEnabled = bEnable;

	if (!bEnable)
	{
		bIsFiring = false;

		bIsAiming = false;
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BaseWalkSpeedCrouched;
	}
}


void UCombatComponent::SetAutoDestroyEquippedWeapon(bool bEnable)
{
	bShouldAutoDestroyEquippedWeapon = bEnable;
}


FVector UCombatComponent::GetRepCameraLocation() const
{
	if (!Character)
	{
		return FVector::ZeroVector;
	}

	if (Character->GetLocalRole() != ENetRole::ROLE_SimulatedProxy)
	{
		if (!FollowCamera)
		{
			return FVector::ZeroVector;
		}

		return FollowCamera->GetComponentLocation();
	}
	else
	{
		return RepCameraLocation;
	}
}


FRotator UCombatComponent::GetRepControlRotation(bool bNormalized) const
{
	if (!Character)
	{
		return FRotator::ZeroRotator;
	}

	if (Character->GetLocalRole() != ENetRole::ROLE_SimulatedProxy)
	{
		if (!bNormalized)
		{
			return Character->GetControlRotation();
		}

		return Character->GetControlRotation().GetNormalized();
	}
	else
	{
		if (!bNormalized)
		{
			return RepControlRotation;
		}

		// Rotators are compressed to 5 bytes and sent through the network. Rotations going from -180 to 180 will be converted to 0 to 360.
		return RepControlRotation.GetNormalized();
	}
}


void UCombatComponent::ApplyMovement(FVector2D InputVector)
{
	if (!bComponentEnabled)
	{
		return;
	}

	if (!Character || !Character->IsLocallyControlled() || InputVector.IsNearlyZero())
	{
		return;
	}

	// Get forward and right movement vectors.
	FRotator ControlRotationYaw(0, Character->GetControlRotation().Yaw, 0);
	FVector ForwardDirection = FRotationMatrix(ControlRotationYaw).GetUnitAxis(EAxis::X);
	FVector RightDirection = FRotationMatrix(ControlRotationYaw).GetUnitAxis(EAxis::Y);

	// Apply movement.
	Character->AddMovementInput(ForwardDirection, InputVector.X);
	Character->AddMovementInput(RightDirection, InputVector.Y);
}


void UCombatComponent::ApplyRotation(FVector2D InputVector)
{
	if (!bComponentEnabled)
	{
		return;
	}

	if (!Character || !Character->IsLocallyControlled() || InputVector.IsNearlyZero())
	{
		return;
	}

	InputVector *= bIsAiming ? AdsSensibilityMultiplier : BaseSensibilityMultiplier;

	Character->AddControllerYawInput(InputVector.X);
	Character->AddControllerPitchInput(-InputVector.Y);

	ServerApplyRotation(InputVector);
}


void UCombatComponent::ServerApplyRotation_Implementation(FVector2D InputVector)
{
	if (!Character->IsLocallyControlled())
	{
		Character->AddControllerYawInput(InputVector.X);
		Character->AddControllerPitchInput(-InputVector.Y);
	}

	RepControlRotation = Character->GetControlRotation();
}


void UCombatComponent::Jump()
{
	if (!bComponentEnabled)
	{
		return;
	}

	if (!Character || !Character->IsLocallyControlled())
	{
		return;
	}

	if (Character->bIsCrouched)
	{
		Character->UnCrouch();
	}
	else
	{
		Character->Jump();
	}
}


void UCombatComponent::StopJumping()
{
	if (!bComponentEnabled)
	{
		return;
	}

	if (!Character || !Character->IsLocallyControlled())
	{
		return;
	}

	Character->StopJumping();
}


void UCombatComponent::ToggleCrouch()
{
	if (!bComponentEnabled)
	{
		return;
	}

	if (!Character || !Character->IsLocallyControlled())
	{
		return;
	}

	// Crouching is already replicated through the CharacterMovementComponent.
	if (Character->bIsCrouched)
	{
		Character->UnCrouch();
	}
	else if (!Character->GetCharacterMovement()->IsFalling())
	{
		Character->Crouch();
	}
}


void UCombatComponent::ReplicateCameraLocation(float DeltaTime)
{
	if (!FollowCamera)
	{
		return;
	}

	if (CameraLocationReplicationTimer <= 0.f)
	{
		FVector CameraLocation = FollowCamera->GetComponentLocation();
		double CameraDistanceSqr = FVector::DistSquared(CameraLocation, LastReplicatedCameraLocation);
		if (CameraDistanceSqr >= CameraLocationReplicationMinDistanceSqr)
		{
			CameraLocationReplicationTimer = 1.f / CameraLocationReplicationFrequency;
			LastReplicatedCameraLocation = CameraLocation;

			RepCameraLocation = CameraLocation;
		}
	}
	else
	{
		CameraLocationReplicationTimer -= DeltaTime;
	}
}


void UCombatComponent::UpdateAimOffset(float DeltaTime)
{
	// ComponentTick function, Character is not nullptr here.

	if (!EquippedWeapon)
	{
		return;
	}

	FVector Velocity = Character->GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = Character->GetCharacterMovement()->IsFalling();

	// Get the replicated ControlRotation and only smooth it for simulated proxies.
	FRotator CurrentControlRotation = GetRepControlRotation();
	if (!Character->IsLocallyControlled())
	{
		SmoothRepControlRotation = FMath::RInterpTo(SmoothRepControlRotation, CurrentControlRotation, DeltaTime,
			SimProxiesControlRotationInterpSpeed);
		CurrentControlRotation = SmoothRepControlRotation;
	}

	// Standing still.
	if (Speed == 0.f && !bIsInAir)
	{
		// Get the delta rotator between the forward yaw of the character (when it stopped) and the current BaseAimRotation yaw.
		FRotator CurrentAimRotation = FRotator(0.f, CurrentControlRotation.Yaw, 0.f);
		AOYaw = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, CachedAOAimRotation).Yaw;

		// If the character is not turning, keep storing the AOYaw in the InterpAOYaw (to later interpolate it towards zero).
		if (TurningInPlace == ETurningInPlace::ETIP_None)
		{
			InterpAOYaw = AOYaw;
		}

		TurnInPlace(DeltaTime);
	}
	else
	{
		// Reset turning in place variables.
		CachedAOAimRotation = FRotator(0.f, CurrentControlRotation.Yaw, 0.f);
		AOYaw = 0.f;

		TurningInPlace = ETurningInPlace::ETIP_None;
	}

	// The pitch must be normalized in the range -180 to 180 for the SimulatedProxies since it is compressed and replicated through the network.
	if (Character->IsLocallyControlled())
	{
		AOPitch = CurrentControlRotation.Pitch;
	}
	else
	{
		AOPitch = CurrentControlRotation.GetNormalized().Pitch;
	}
}


void UCombatComponent::TurnInPlace(float DeltaTime)
{
	// Turn right or left.
	if (AOYaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AOYaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	// If the character is turning in place (left or right), interpolate the AOYaw towards zero (making a 90° turn).
	if (TurningInPlace != ETurningInPlace::ETIP_None)
	{
		InterpAOYaw = FMath::FInterpConstantTo(InterpAOYaw, 0.f, DeltaTime, TurnInPlaceSpeed);
		AOYaw = InterpAOYaw;

		// If the turn has been completed and the character is staying still, cache the current BaseAimRotation yaw.
		if (FMath::Abs(AOYaw) == 0.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_None;
			CachedAOAimRotation = FRotator(0.f, Character->IsLocallyControlled() ? Character->GetControlRotation().Yaw :
				SmoothRepControlRotation.Yaw, 0.f);
		}
	}
}


void UCombatComponent::TraceCameraTargetPoint()
{
	// ComponentTick function, Character is not nullptr here.

	UWorld *World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector Start = GetRepCameraLocation();
	FVector CameraForward = GetRepControlRotation(false).Vector();
	FVector End = Start + MaxCameraTraceDistance * CameraForward;

	FCollisionQueryParams CameraTraceParams;
	CameraTraceParams.AddIgnoredActor(Character);
	if (EquippedWeapon)
	{
		CameraTraceParams.AddIgnoredActor(EquippedWeapon);
	}

	World->LineTraceSingleByChannel(CameraTraceHitResult, Start, End, ECollisionChannel::ECC_Visibility,
		CameraTraceParams);

	bool bPossibleTargetDetected = false;
	if (CameraTraceHitResult.bBlockingHit)
	{
		// Prepare vectors to be used in the Dot function by also projecting them on the XY plane.
		FVector CharacterLocation = Character->GetActorLocation();
		CameraForward.Z = 0.f;
		CameraForward.Normalize();

		FVector TargetVector = CameraTraceHitResult.ImpactPoint - CharacterLocation;
		TargetVector.Z = 0.f;
		TargetVector.Normalize();

		float Dot = FVector::DotProduct(CameraForward, TargetVector);
		// If the target is outside of my target trace cone.
		if (Dot >= CameraTraceTargetDotThreshold)
		{
			bPossibleTargetDetected = true;
		}
		else
		{
			CameraTraceHitResult.Reset();
			CameraTraceHitResult.ImpactPoint = End;
		}
	}
	else
	{
		CameraTraceHitResult.ImpactPoint = End;
	}

	// Change crosshair color only on locally controlled characters.
	if (Character->IsLocallyControlled())
	{
		bool bIsValidTarget = bPossibleTargetDetected && CameraTraceHitResult.GetActor() &&
			CameraTraceHitResult.GetActor()->Implements<UDamageableInterface>();

		if (bAimingAtTarget != bIsValidTarget)
		{
			bAimingAtTarget = bIsValidTarget;

			OnCrosshairColorChanged.Broadcast(bAimingAtTarget ? TargetCrosshairColor : DefaultCrosshairColor);
		}
	}
}


void UCombatComponent::UpdateCharacterOcclusion()
{
	// ComponentTick function, Character is not nullptr here. Runs on locally controlled actors only.

	if (!FollowCamera)
	{
		return;
	}

	bool bIsZoomingIn = EquippedWeapon && EquippedWeapon->GetWeaponAimMode() == EWeaponAimMode::EWAM_ZoomIn &&
		AimValue > ZoomInAimValueOcclusionThreshold;

	// Prioritize hiding meshes while a zoom in scope is active.
	bool bHideMeshes;
	if (bIsZoomingIn)
	{
		bHideMeshes = true;
	}
	else
	{
		FVector CameraVector = FollowCamera->GetComponentLocation() - Character->GetActorLocation();
		bHideMeshes = CameraVector.SizeSquared() < CameraOcclusionSqrDistance;
	}

	Character->GetMesh()->SetVisibility(!bHideMeshes);
	if (EquippedWeapon)
	{
		EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = bHideMeshes;
	}
}


void UCombatComponent::UpdateWeaponSpread(float DeltaTime)
{
	// ComponentTick function, Character is not nullptr here. Runs on locally controlled actors only.

	float SpreadInterpSpeed = EquippedWeapon ? EquippedWeapon->GetCrosshairSpreadInterpSpeed() : DefaultAimInterpSpeed;

	FVector2D WalkSpeedRange;
	if (Character->bIsCrouched)
	{
		WalkSpeedRange = FVector2D(0.f, Character->GetCharacterMovement()->MaxWalkSpeedCrouched);
	}
	else
	{
		WalkSpeedRange = FVector2D(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
	}

	FVector Velocity = Character->GetVelocity();
	Velocity.Z = 0.f;
	float GroundNormalizedSpread = FMath::GetMappedRangeValueClamped(WalkSpeedRange, FVector2D::UnitY(), Velocity.Size());
	CrosshairGroundSpread = FMath::FInterpConstantTo(CrosshairGroundSpread, GroundNormalizedSpread, DeltaTime, SpreadInterpSpeed);

	if (Character->GetMovementComponent()->IsFalling())
	{
		CrosshairAirSpread = FMath::FInterpConstantTo(CrosshairAirSpread, 1.f, DeltaTime, SpreadInterpSpeed);
	}
	else
	{
		CrosshairAirSpread = FMath::FInterpConstantTo(CrosshairAirSpread, 0.f, DeltaTime, SpreadInterpSpeed);
	}

	CrosshairFiringSpread = FMath::FInterpConstantTo(CrosshairFiringSpread, 0.f, DeltaTime, SpreadInterpSpeed);
	CrosshairSpread = FMath::Clamp(FMath::Max3(CrosshairGroundSpread, CrosshairAirSpread, CrosshairFiringSpread), 0.f, 1.f);

	// Broadcast crosshair spread changes.
	if (CrosshairSpread != LastCrosshairSpread)
	{
		LastCrosshairSpread = CrosshairSpread;
		OnCrosshairSpreadChanged.Broadcast(CrosshairSpread);
	}
}


void UCombatComponent::UpdateAimValue(float DeltaTime)
{
	// ComponentTick function, Character is not nullptr here. Runs on locally controlled actors only.

	float TargetFOV = bIsAiming ? LastEquippedWeaponAimCameraFOV : DefaultCameraFOV;

	if (CurrentCameraFOV != TargetFOV)
	{
		float AimInterpSpeed = EquippedWeapon ? EquippedWeapon->GetAimInterpSpeed() : DefaultAimInterpSpeed;
		CurrentCameraFOV = FMath::FInterpConstantTo(CurrentCameraFOV, TargetFOV, DeltaTime, AimInterpSpeed);
		if (IsValid(FollowCamera))
		{
			FollowCamera->SetFieldOfView(CurrentCameraFOV);
		}

		// Notify the AimValue changes (primarily to the UI which is listening).
		LastAimValue = AimValue;
		AimValue = FMath::GetRangePct(DefaultCameraFOV, LastEquippedWeaponAimCameraFOV, CurrentCameraFOV);
		OnAimValueChanged.Broadcast(AimValue);

		CalculateAimState();
	}
}


void UCombatComponent::CalculateAimState()
{
	if (AimValue > LastAimValue)
	{
		if (AimState == EAimState::EAS_ZoomOutStarted || LastAimValue == 0.f)
		{
			AimState = EAimState::EAS_ZoomInStarted;
			OnAimStateChanged.Broadcast(AimState);
		}
		else if (AimValue == 1.f)
		{
			AimState = EAimState::EAS_ZoomInCompleted;
			OnAimStateChanged.Broadcast(AimState);
		}
	}
	else if (AimValue < LastAimValue)
	{
		if (AimState == EAimState::EAS_ZoomInStarted || LastAimValue == 1.f)
		{
			AimState = EAimState::EAS_ZoomOutStarted;
			OnAimStateChanged.Broadcast(AimState);
		}
		else if (AimValue == 0.f)
		{
			AimState = EAimState::EAS_ZoomOutCompleted;
			OnAimStateChanged.Broadcast(AimState);
		}
	}
}


void UCombatComponent::StopAllActions()
{
	bIsFiring = false;

	bIsAiming = false;
	Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BaseWalkSpeedCrouched;

	TryCancelingReload();
}


void UCombatComponent::SpawnStartingWeapon()
{
	if (!StartingWeaponClass)
	{
		return;
	}

	UWorld *World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AWeaponBase *StartingWeapon = World->SpawnActor<AWeaponBase>(
		StartingWeaponClass,
		Character->GetActorLocation(),
		FRotator::ZeroRotator,
		SpawnParams
	);

	EquipWeapon(StartingWeapon);
}


void UCombatComponent::SetOverlappingWeapon(AWeaponBase *Weapon)
{
	// This function is though out to only run on the server since weapon overlaps should only be enabled on it.

	if (!IsValid(Character))
	{
		return;
	}

	// This check will run on the pawn controlled by the server.
	if (Character->IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->TogglePickupWidget(false);
		}

		if (Weapon)
		{
			Weapon->TogglePickupWidget(true);
		}
	}

	// OverlappingWeapon is RepNotify (OwnerOnly).
	OverlappingWeapon = Weapon;
}


void UCombatComponent::OnRep_OverlappingWeapon(AWeaponBase *LastWeapon)
{
	// OnRepNotify doesn't get called on the server. Its input parameter returns the last variable value before getting replicated.

	if (LastWeapon)
	{
		LastWeapon->TogglePickupWidget(false);
	}

	if (OverlappingWeapon)
	{
		OverlappingWeapon->TogglePickupWidget(true);
	}
}


void UCombatComponent::TryPickingUpWeapon()
{
	if (!bComponentEnabled)
	{
		return;
	}

	if (!IsValid(Character))
	{
		return;
	}

	if (IsValid(OverlappingWeapon))
	{
		ServerTryPickingUpWeapon();
	}
}


void UCombatComponent::ServerTryPickingUpWeapon_Implementation()
{
	if (IsValid(OverlappingWeapon))
	{
		EquipWeapon(OverlappingWeapon);
	}
}


void UCombatComponent::EquipWeapon(AWeaponBase *Weapon)
{
	// EquipWeapon needs to be called from the server (to prevent clients from possibly cheating).

	if (!IsValid(Character))
	{
		return;
	}

	if (IsValid(EquippedWeapon))
	{
		TryCancelingReload();

		FVector DropDirection = GetRepControlRotation().Vector();
		EquippedWeapon->Drop(DropDirection);
	}

	bool bIsValidWeapon = IsValid(Weapon);

	// EquippedWeapon is RepNotify.
	EquippedWeapon = Weapon;
	if (bIsValidWeapon)
	{
		EquippedWeapon->SetOwner(Character);
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		LastEquippedWeaponAimCameraFOV = EquippedWeapon->GetAimCameraFOV();

		const USkeletalMeshSocket *HandSocket = Character->GetMesh()->GetSocketByName(RightHandSocketName);
		if (HandSocket)
		{
			// AttachActor() is already a replicated function.
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}

		if (EquippedWeapon->GetPickupSound())
		{
			UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->GetPickupSound(), Character->GetActorLocation());
		}

		// Autoreloading on pickup.
		if (EquippedWeapon->NeedsReloading())
		{
			Reload();
		}
	}
	else
	{
		StopAllActions();
	}

	// Set strafing movement (it will be updated in OnRep_EquippedWeapon() for the other clients.
	Character->GetCharacterMovement()->bOrientRotationToMovement = !bIsValidWeapon;
	Character->bUseControllerRotationYaw = bIsValidWeapon;

	OnEquippedWeaponChanged.Broadcast(this);
}


void UCombatComponent::OnRep_EquippedWeapon()
{
	if (!IsValid(Character))
	{
		bStartingWeaponSpawnFailed = true;
		return;
	}

	bool bIsValidWeapon = IsValid(EquippedWeapon);

	if (bIsValidWeapon)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		LastEquippedWeaponAimCameraFOV = EquippedWeapon->GetAimCameraFOV();

		const USkeletalMeshSocket *HandSocket = Character->GetMesh()->GetSocketByName(RightHandSocketName);
		if (HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}

		if (EquippedWeapon->GetPickupSound())
		{
			UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->GetPickupSound(), Character->GetActorLocation());
		}
	}
	else
	{
		StopAllActions();
	}

	// Strafing is enabled when a new weapon is set through RepNotify.
	Character->GetCharacterMovement()->bOrientRotationToMovement = !bIsValidWeapon;
	Character->bUseControllerRotationYaw = bIsValidWeapon;

	OnEquippedWeaponChanged.Broadcast(this);
}


void UCombatComponent::DropWeapon()
{
	if (!IsValid(Character) || !IsValid(EquippedWeapon))
	{
		return;
	}

	if (Character->HasAuthority())
	{
		EquipWeapon(nullptr);
	}
	else
	{
		ServerDropWeapon();
	}
}


void UCombatComponent::ServerDropWeapon_Implementation()
{
	if (!IsValid(Character) || !IsValid(EquippedWeapon))
	{
		return;
	}

	EquipWeapon(nullptr);
}


void UCombatComponent::ToggleAim(bool bAim)
{
	if (!bComponentEnabled)
	{
		return;
	}

	if (!IsValid(Character) || !EquippedWeapon || bIsAiming == bAim)
	{
		return;
	}

	// Set bIsAiming immediately (specifically for clients) for cosmetic purposes.
	bIsAiming = bAim;
	Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AdsWalkSpeed : BaseWalkSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = bIsAiming ? AdsWalkSpeedCrouched : BaseWalkSpeedCrouched;

	// Only set the previous values from client to server.
	if (!Character->HasAuthority())
	{
		ServerToggleAim(bAim);
	}
}


void UCombatComponent::ServerToggleAim_Implementation(bool bAim)
{
	// bIsAiming is Replicated.
	bIsAiming = bAim;

	if (IsValid(Character))
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AdsWalkSpeed : BaseWalkSpeed;
	}
}


void UCombatComponent::ToggleFiring(bool bFire)
{
	if (!bComponentEnabled)
	{
		return;
	}

	bIsFiring = bFire;

	if (bIsFiring)
	{
		Fire();
	}
}


bool UCombatComponent::CanFire()
{
	return IsValid(Character) && IsValid(EquippedWeapon) && bCanFire && CombatState == ECombatState::ECS_None;
}


void UCombatComponent::Fire()
{
	if (!IsValid(Character) || !IsValid(EquippedWeapon) || CombatState != ECombatState::ECS_None)
	{
		return;
	}

	// Attempt to autoreload.
	if (EquippedWeapon->NeedsReloading())
	{
		Reload();

		return;
	}

	if (bCanFire && EquippedWeapon->GetMagazineAmmo() > 0)
	{
		bCanFire = false;

		if (Character->HasAuthority())
		{
			MulticastFire(CameraTraceHitResult.ImpactPoint);
		}
		else
		{
			ServerFire(CameraTraceHitResult.ImpactPoint);
		}

		StartFiringCooldown();
		CrosshairFiringSpread = FMath::Clamp(CrosshairFiringSpread + EquippedWeapon->GetCrosshairFiringSpread(), 0.f, 1.f);
	}
}


void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize &Target)
{
	if (!CanFire())
	{
		return;
	}

	if (Target.Equals(CameraTraceHitResult.ImpactPoint, ClientsTargetOffsetTolerance))
	{
		MulticastFire(Target);
	}
	else
	{
		MulticastFire(CameraTraceHitResult.ImpactPoint);
	}

	StartFiringCooldown();
}


void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize &Target)
{
	if (IsValid(Character) && IsValid(EquippedWeapon) && CombatState == ECombatState::ECS_None)
	{
		// Play the firing animation montage and fire the weapon.
		UAnimInstance *AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance && FireWeaponMontage)
		{
			AnimInstance->Montage_Play(FireWeaponMontage);
			AnimInstance->Montage_JumpToSection(bIsAiming ? FireWeaponAdsMontageSectionName : FireWeaponHipMontageSectionName);

			EquippedWeapon->Fire(Target, AimValue);
		}
	}
}


void UCombatComponent::StartFiringCooldown()
{
	if (!IsValid(Character) || !EquippedWeapon)
	{
		return;
	}

	float FiringDelay = EquippedWeapon->GetFiringDelay();
	if (FiringDelay > 0.f)
	{
		Character->GetWorldTimerManager().SetTimer(FiringTimerHandle, this, &UCombatComponent::FiringCooldownFinished, FiringDelay);
	}
	else
	{
		FiringCooldownFinished();
	}
}


void UCombatComponent::FiringCooldownFinished()
{
	bCanFire = true;

	if (!EquippedWeapon)
	{
		return;
	}

	if (bIsFiring && EquippedWeapon->IsAutomatic())
	{
		Fire();
	}
}


void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_None:
	{
		// Stop the reloading animation.
		UAnimInstance *AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Stop(.1f, LastReloadMontage);
		}

		if (bIsFiring) // Reloading may not have occurred yet, will fix later <-----------
		{
			Fire();
		}
		break;
	}

	case ECombatState::ECS_Reloading:
		PlayReloadMontage();
		break;
	}
}


bool UCombatComponent::CanReload()
{
	return IsValid(EquippedWeapon) && EquippedWeapon->CanReload() && CombatState == ECombatState::ECS_None;
}


void UCombatComponent::Reload()
{
	if (!bComponentEnabled)
	{
		return;
	}

	if (!CanReload())
	{
		return;
	}

	if (IsValid(Character) && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Reloading;
		PlayReloadMontage();
	}
	else
	{
		ServerReload();
	}
}


void UCombatComponent::ServerReload_Implementation()
{
	if (!CanReload())
	{
		return;
	}

	CombatState = ECombatState::ECS_Reloading;
	PlayReloadMontage();
}


void UCombatComponent::PlayReloadMontage()
{
	if (!IsValid(Character) || !EquippedWeapon)
	{
		return;
	}

	// Get the corresponding weapon reload montage and play it.
	UAnimInstance *AnimInstance = Character->GetMesh()->GetAnimInstance();
	USkeletalMeshComponent *SkeletalMeshComp = Character->GetMesh();
	if (SkeletalMeshComp)
	{
		LastReloadMontage = EquippedWeapon->GetReloadMontage();
		if (AnimInstance && LastReloadMontage)
		{
			AnimInstance->Montage_Play(LastReloadMontage);

			// Avoid getting stuck in the reloading combat state if the reload montage gets interrupted.
			if (Character->HasAuthority())
			{
				Character->GetWorldTimerManager().SetTimer(ResetReloadTimerHandle, this, &UCombatComponent::ResetReload,
					LastReloadMontage->GetPlayLength(), false);
			}
		}
	}
}


void UCombatComponent::CompleteReload()
{
	if (IsValid(Character) && Character->HasAuthority())
	{
		if (EquippedWeapon)
		{
			EquippedWeapon->Reload();
		}

		CombatState = ECombatState::ECS_None;

		// Autofire after having reloaded.
		if (bIsFiring)
		{
			Fire();
		}
	}
}


void UCombatComponent::TryCancelingReload()
{
	// If reloading, cancel the reload on the server.
	if (CombatState == ECombatState::ECS_Reloading)
	{
		// Stop the reloading animation.
		UAnimInstance *AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Stop(.1f, LastReloadMontage);
		}

		CombatState = ECombatState::ECS_None;
		ResetReloadTimerHandle.Invalidate();
	}
}


void UCombatComponent::ResetReload()
{
	if (CombatState == ECombatState::ECS_Reloading)
	{
		CombatState = ECombatState::ECS_None;
	}
}


void UCombatComponent::OnPlayerEliminated(AController *AttackerController, AController *EliminatedController, AActor *DamageCauser)
{
	// Disable movement.
	UCharacterMovementComponent *CharacterMovement = Character->GetCharacterMovement();
	if (CharacterMovement)
	{
		CharacterMovement->DisableMovement();
		CharacterMovement->StopMovementImmediately();
	}

	// Disable inputs.
	if (PlayerController)
	{
		Character->DisableInput(PlayerController);
	}

	// Disable collisions.
	Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Character->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (Character->HasAuthority())
	{
		DropWeapon();
	}
}
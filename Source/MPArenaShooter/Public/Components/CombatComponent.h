// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/TurningInPlace.h"
#include "Types/CrosshairTextures.h"
#include "Types/CombatState.h"
#include "CombatComponent.generated.h"

class ACharacterCore;
class AWeaponBase;
class APlayerControllerCore;
class UTexture2D;
class AHUDCore;
class UCameraComponent;
class USpringArmComponent;


UENUM(BlueprintType)
enum class EAimState : uint8
{
	EAS_ZoomInStarted		UMETA(DisplayName = "Zoom In Started"),
	//EAS_ZoomInCanceled		UMETA(DisplayName = "Zoom In Canceled"),
	EAS_ZoomInCompleted		UMETA(DisplayName = "Zoom In Completed"),
	EAS_ZoomOutStarted		UMETA(DisplayName = "Zoom Out Started"),
	//EAS_ZoomOutCanceled		UMETA(DisplayName = "Zoom Out Canceled"),
	EAS_ZoomOutCompleted	UMETA(DisplayName = "Zoom Out Completed"),

	EAS_MAX					UMETA(Hidden)
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAimValueChangedSignature, float, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAimStateChangedSignature, EAimState, AimState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquippedWeaponChangedSignature, UCombatComponent *, CombatComp);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrosshairSpreadChangedSignature, float, Spread);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrosshairColorChangedSignature, FLinearColor, Color);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponMagazineAmmoChangedSignature, int32, MagazineAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponReserveAmmoChangedSignature, int32, ReserveAmmo);


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MPARENASHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FAimValueChangedSignature OnAimValueChanged;

	UPROPERTY(BlueprintAssignable)
	FAimStateChangedSignature OnAimStateChanged;

	UPROPERTY(BlueprintAssignable)
	FOnEquippedWeaponChangedSignature OnEquippedWeaponChanged;

	UPROPERTY(BlueprintAssignable)
	FOnCrosshairSpreadChangedSignature OnCrosshairSpreadChanged;

	UPROPERTY(BlueprintAssignable)
	FOnCrosshairColorChangedSignature OnCrosshairColorChanged;


	UCombatComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;


	FORCEINLINE FName GetRightHandSocketName() const { return RightHandSocketName; }

	FORCEINLINE FName GetRightHandBoneName() const { return RightHandBoneName; }

	FORCEINLINE FName GetLeftHandSocketName() const { return LeftHandSocketName; }


	FVector GetRepCameraLocation() const;

	FRotator GetRepControlRotation(bool bNormalized = true) const;

	FORCEINLINE float GetAOYaw() const { return AOYaw; }

	FORCEINLINE float GetAOPitch() const { return AOPitch; }

	FORCEINLINE float GetLeaningSpeed() const { return LeaningSpeed; }

	FORCEINLINE FVector2D GetLeaningClamp() const { return LeaningClamp; }

	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }

	FORCEINLINE FVector GetCameraTraceHitTarget() const { return CameraTraceHitResult.ImpactPoint; }

	FORCEINLINE const FCrosshairTextures &GetDefaultCrosshairTextures() const { return DefaultCrosshairTextures; }

	FORCEINLINE float GetBaseWalkReferenceSpeed() const { return BaseWalkReferenceSpeed; }

	FORCEINLINE float GetAdsWalkReferenceSpeed() const { return AdsWalkReferenceSpeed; }

	FORCEINLINE float GetBaseWalkReferenceSpeedCrouched() const { return BaseWalkReferenceSpeedCrouched; }

	FORCEINLINE float GetAdsWalkReferenceSpeedCrouched() const { return AdsWalkReferenceSpeedCrouched; }

	FORCEINLINE float GetRotateWeaponToTargetSpeed() const { return RotateWeaponToTargetSpeed; }


	FORCEINLINE AWeaponBase *GetEquippedWeapon() const { return EquippedWeapon; }

	FORCEINLINE bool HasWeaponEquipped() const { return EquippedWeapon != nullptr; }

	FORCEINLINE bool IsAiming() const { return bIsAiming; }

	FORCEINLINE float GetAimValue() const { return AimValue; }

	FORCEINLINE EAimState GetAimState() const { return AimState; }

	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }


	UFUNCTION(BlueprintCallable)
	void SetComponentEnabled(bool bEnable);

	UFUNCTION(BlueprintCallable)
	void SetAutoDestroyEquippedWeapon(bool bEnable);

	UFUNCTION(BlueprintCallable)
	void ApplyMovement(FVector2D InputVector);

	UFUNCTION(BlueprintCallable)
	void ApplyRotation(FVector2D InputVector);

	UFUNCTION(BlueprintCallable)
	void Jump();

	UFUNCTION(BlueprintCallable)
	void StopJumping();

	UFUNCTION(BlueprintCallable)
	void ToggleCrouch();

	UFUNCTION(BlueprintCallable)
	void SetOverlappingWeapon(AWeaponBase *Weapon);

	UFUNCTION(BlueprintCallable)
	void TryPickingUpWeapon();

	UFUNCTION(BlueprintCallable)
	void DropWeapon();

	UFUNCTION(BlueprintCallable)
	void ToggleAim(bool bAim);

	UFUNCTION(BlueprintCallable)
	void ToggleFiring(bool bFire);

	UFUNCTION(BlueprintCallable)
	void Reload();

	UFUNCTION(BlueprintCallable)
	void CompleteReload();


protected:

	bool bComponentEnabled = true;


	virtual void BeginPlay() override;


	virtual void PostBeginPlay();


private:

	ACharacter *Character = nullptr;
	APlayerController *PlayerController = nullptr;


	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<AWeaponBase> StartingWeaponClass;


	UPROPERTY(EditDefaultsOnly, Category = "Combat|Mesh")
	FName RightHandSocketName = FName("RightHandSocket");

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Mesh")
	FName LeftHandSocketName = FName("LeftHandSocket");

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Mesh")
	FName RightHandBoneName = FName("hand_r");


	UPROPERTY(EditAnywhere, Category = "Combat|Movement")
	float BaseWalkSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Movement")
	float AdsWalkSpeed = 350.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Movement")
	float BaseWalkSpeedCrouched = 350.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Movement")
	float AdsWalkSpeedCrouched = 300.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Movement")
	float LeaningSpeed = 1.2f;

	UPROPERTY(EditAnywhere, Category = "Combat|Movement")
	FVector2D LeaningClamp = FVector2D(-65.f, 65.f);

	UPROPERTY(EditAnywhere, Category = "Combat|Movement")
	float TurnInPlaceSpeed = 200.f;


	UPROPERTY(EditAnywhere, Category = "Combat|Camera")
	float DefaultCameraFOV = 90.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Camera")
	float MaxCameraTraceDistance = 50000.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Camera")
	float CameraTraceConeDegrees = 90.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Camera")
	float DefaultFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Camera")
	float BaseSensibilityMultiplier = 1.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Camera")
	float AdsSensibilityMultiplier = .33f;

	UPROPERTY(EditAnywhere, Category = "Combat|Camera")
	float MaxViewPitch = 89.75f;

	UPROPERTY(EditAnywhere, Category = "Combat|Camera")
	float CameraOcclusionDistance = 200.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Camera", meta = (ClampMin = "0", ClampMax = "1"))
	float ZoomInAimValueOcclusionThreshold = .5f;

	UPROPERTY(EditAnywhere, Category = "Combat|Camera", meta = (ClampMin = "0"))
	float DefaultAimInterpSpeed = 350.f;


	UPROPERTY(EditAnywhere, Category = "Combat|User Interface")
	FCrosshairTextures DefaultCrosshairTextures;

	UPROPERTY(EditAnywhere, Category = "Combat|User Interface")
	FLinearColor DefaultCrosshairColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "Combat|User Interface")
	FLinearColor TargetCrosshairColor = FLinearColor::Red;


	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	float BaseWalkReferenceSpeed = 350.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	float AdsWalkReferenceSpeed = 200.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	float BaseWalkReferenceSpeedCrouched = 200.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	float AdsWalkReferenceSpeedCrouched = 200.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	float RotateWeaponToTargetSpeed = 15.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	UAnimMontage *FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	FName FireWeaponHipMontageSectionName = FName("Hip");

	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	FName FireWeaponAdsMontageSectionName = FName("Ads");


	UPROPERTY(EditAnywhere, Category = "Combat|Networking", meta = (ClampMin = "0"))
	float CameraLocationReplicationMinDistance = 5.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Networking", meta = (ClampMin = "0"))
	float CameraLocationReplicationFrequency = 20.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Networking", meta = (ClampMin = "0"))
	float SimProxiesControlRotationInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Networking", meta = (ClampMin = "0"))
	double ClientsTargetOffsetTolerance = 25.;


	bool bShouldAutoDestroyEquippedWeapon = false;

	bool bStartingWeaponSpawnFailed = false;

	// FollowCamera.
	UPROPERTY(Replicated)
	FVector_NetQuantize RepCameraLocation;
	float CameraLocationReplicationTimer;
	FVector LastReplicatedCameraLocation;
	double CameraLocationReplicationMinDistanceSqr;

	UPROPERTY(Replicated)
	FRotator RepControlRotation;
	FRotator SmoothRepControlRotation;

	// Aim offset.
	float AOYaw = 0.f;
	float AOPitch = 0.f;
	FRotator CachedAOAimRotation = FRotator::ZeroRotator;

	// Turning in place.
	ETurningInPlace TurningInPlace = ETurningInPlace::ETIP_None;
	float InterpAOYaw = 0.f;

	// Camera trace and occlusion.
	float CameraOcclusionSqrDistance;
	FHitResult CameraTraceHitResult;
	float CameraTraceTargetDotThreshold;
	bool bAimingAtTarget = false;

	// Crosshair spread.
	float CrosshairSpread = 0.f;
	float CrosshairGroundSpread = 0.f;
	float CrosshairAirSpread = 0.f;
	float CrosshairFiringSpread = 0.f;
	float LastCrosshairSpread = FLT_MAX;

	// Aiming down sights.
	UPROPERTY()
	UCameraComponent *FollowCamera;
	float AimValue;
	float LastAimValue;
	EAimState AimState = EAimState::EAS_ZoomOutCompleted;
	float CurrentCameraFOV;
	float LastCameraFOV;
	float LastEquippedWeaponAimCameraFOV;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeaponBase *OverlappingWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon);
	AWeaponBase *EquippedWeapon;

	UPROPERTY(Replicated)
	bool bIsAiming = false;

	// Firing.
	bool bIsFiring = false;
	bool bCanFire = true;
	FTimerHandle FiringTimerHandle;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState);
	ECombatState CombatState = ECombatState::ECS_None;

	// Reloading.
	UPROPERTY()
	UAnimMontage *LastReloadMontage;
	FTimerHandle ResetReloadTimerHandle;


	UFUNCTION(Server, Unreliable)
	void ServerApplyRotation(FVector2D InputVector);

	void ReplicateCameraLocation(float DeltaTime);

	void UpdateAimOffset(float DeltaTime);

	void TurnInPlace(float DeltaTime);

	void TraceCameraTargetPoint();

	void UpdateCharacterOcclusion();

	void UpdateWeaponSpread(float DeltaTime);

	void UpdateAimValue(float DeltaTime);

	void CalculateAimState();

	void StopAllActions();

	void SpawnStartingWeapon();

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeaponBase *LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerTryPickingUpWeapon();

	void EquipWeapon(AWeaponBase *Weapon);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION(Server, Reliable)
	void ServerDropWeapon();

	UFUNCTION(Server, Reliable)
	void ServerToggleAim(bool bAim);


	bool CanFire();

	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize &Target);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize &Target);

	void StartFiringCooldown();

	void FiringCooldownFinished();


	UFUNCTION()
	void OnRep_CombatState();


	bool CanReload();

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void PlayReloadMontage();

	void TryCancelingReload();

	void ResetReload();


	UFUNCTION()
	void OnPlayerEliminated(AController *AttackerController, AController *EliminatedController, AActor *DamageCauser);
};
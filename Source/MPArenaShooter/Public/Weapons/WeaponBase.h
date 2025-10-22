// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/CrosshairTextures.h"
#include "Types/Rarity.h"
#include "Interfaces/KillfeedInterface.h"
#include "WeaponBase.generated.h"

class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;
class AAmmoCasing;
class UTexture2D;
class UZoomInScopeWidget;


UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Interactable	UMETA(DisplayName = "Interactable"),
	EWS_Equipped		UMETA(DisplayName = "Equipped"),
	EWS_Dropped			UMETA(DisplayName = "Dropped"),

	EWS_MAX				UMETA(Hidden)
};


UENUM(BlueprintType)
enum class EWeaponAimMode : uint8
{
	EWAM_AimDownSights	UMETA(DisplayName = "Aim Down Sight"),
	EWAM_ZoomIn			UMETA(DisplayName = "Zoom In"),

	EWAM_MAX			UMETA(Hidden)
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMagazineAmmoChangedSignature, int32, MagazineAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReserveAmmoChangedSignature, int32, ReserveAmmo);


UCLASS()
class MPARENASHOOTER_API AWeaponBase : public AActor, public IKillfeedInterface
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FOnMagazineAmmoChangedSignature OnMagazineAmmoChanged;

	UPROPERTY(BlueprintAssignable)
	FOnReserveAmmoChangedSignature OnReserveAmmoChanged;


	AWeaponBase();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;
#endif

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

	virtual void SetOwner(AActor *NewOwner) override;


	virtual UTexture2D *GetKillfeedIcon() const override;


	FORCEINLINE USphereComponent *GetWeaponOverlap() const { return WeaponOverlap; }

	FORCEINLINE USkeletalMeshComponent *GetWeaponMesh() const { return WeaponMesh; }

	FORCEINLINE EWeaponAimMode GetWeaponAimMode() const { return AimMode; }

	FORCEINLINE TSubclassOf<UZoomInScopeWidget> GetWeaponZoomInScopeWidget() const { return ZoomInScopeWidget; }

	FORCEINLINE float GetAimCameraFOV() const { return AimCameraFOV; }

	FORCEINLINE float GetAimInterpSpeed() const { return AimInterpSpeed; }

	FORCEINLINE float GetZoomInInterpSpeedMultiplier() const { return ZoomInInterpSpeedMultiplier; }

	FORCEINLINE bool IsAutomatic() const { return bIsAutomatic; }

	FORCEINLINE float GetFiringDelay() const { return FiringDelay; }

	FORCEINLINE float GetDamage() const { return Damage; }

	FORCEINLINE bool CanReload() const { return MagazineAmmo < MagazineSize && ReserveAmmo > 0; }

	FORCEINLINE bool NeedsReloading() const { return MagazineAmmo == 0 && ReserveAmmo > 0; }

	FORCEINLINE int32 GetMagazineAmmo() const { return MagazineAmmo; }

	FORCEINLINE int32 GetReserveAmmo() const { return ReserveAmmo; }

	FORCEINLINE bool HasNoAmmo() const { return MagazineAmmo == 0 && ReserveAmmo == 0; }

	FORCEINLINE const FCrosshairTextures &GetCrosshairTextures() const { return CrosshairTextures; }

	FORCEINLINE FVector2D GetCrosshairSpreadRange() const { return CrosshairSpreadRange; }

	FORCEINLINE FVector2D GetCrosshairAimSpreadRange() const { return CrosshairAimSpreadRange; }

	FORCEINLINE float GetCrosshairFiringSpread() const { return CrosshairFiringSpread; }

	FORCEINLINE float GetCrosshairSpreadInterpSpeed() const { return CrosshairSpreadInterpSpeed; }

	FORCEINLINE UAnimMontage *GetReloadMontage() const { return ReloadMontage; }

	FORCEINLINE USoundBase *GetPickupSound() const { return PickupSound; }


	void TogglePickupWidget(bool bEnable);

	void SetWeaponState(EWeaponState State);

	void Fire(const FVector &Target, float AimValue);

	void Drop(FVector Direction);

	void Reload();


protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USkeletalMeshComponent *WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent *WeaponOverlap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent *PickupWidget;


	UPROPERTY(EditDefaultsOnly, Category = "Combat|Mesh")
	FName MuzzleSocketName = FName("MuzzleFlash");

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Mesh")
	FName EjectionPortSocketName = FName("AmmoEject");

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Mesh")
	bool bSupportsMeshPhysics = false;


	UPROPERTY(EditAnywhere, Category = "Combat|Stats")
	ERarity WeaponRarity = ERarity::ER_Common;

	UPROPERTY(EditAnywhere, Category = "Combat|Stats")
	bool bIsAutomatic = true;

	UPROPERTY(EditAnywhere, Category = "Combat|Stats")
	float FiringDelay = .15f;

	UPROPERTY(EditAnywhere, Category = "Combat|Stats")
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Stats")
	int32 StartingAmmo = 120;

	UPROPERTY(EditAnywhere, Category = "Combat|Stats")
	int32 MagazineSize = 30;

	UPROPERTY(EditAnywhere, Category = "Combat|Stats")
	float MaxSpreadAngle = 0.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Stats")
	float MaxAimSpreadAngle = 0.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Stats")
	float MaxRange = 50000.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Stats")
	float DropForce = 250.f;


	UPROPERTY(EditAnywhere, Category = "Combat|Camera")
	EWeaponAimMode AimMode = EWeaponAimMode::EWAM_AimDownSights;

	UPROPERTY(EditAnywhere, Category = "Combat|Camera")
	float AimCameraFOV = 50.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Camera")
	float AimInterpSpeed = 350.f;

	UPROPERTY(EditAnywhere, Category = "Combat|Camera", meta = (EditCondition = "AimMode == EWeaponAimMode::EWAM_ZoomIn",
		EditConditionHides))
	TSubclassOf<UZoomInScopeWidget> ZoomInScopeWidget;


	UPROPERTY(EditAnywhere, Category = "Combat|User Interface")
	UTexture2D *KillfeedIcon;

	UPROPERTY(EditAnywhere, Category = "Combat|User Interface")
	FCrosshairTextures CrosshairTextures;

	UPROPERTY(EditAnywhere, Category = "Combat|User Interface")
	FVector2D CrosshairSpreadRange = FVector2D(15.f, 50.f);

	UPROPERTY(EditAnywhere, Category = "Combat|User Interface")
	FVector2D CrosshairAimSpreadRange = FVector2D(5.f, 20.f);

	UPROPERTY(EditAnywhere, Category = "Combat|User Interface")
	float CrosshairFiringSpread = .6f;

	UPROPERTY(EditAnywhere, Category = "Combat|User Interface")
	float CrosshairSpreadInterpSpeed = 5.f;


	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	UAnimationAsset *FiringAnimation;

	UPROPERTY(EditAnywhere, Category = "Combat|Animations")
	UAnimMontage *ReloadMontage;


	UPROPERTY(EditAnywhere, Category = "Combat|VFX")
	TSubclassOf<AAmmoCasing> EjectedCasingClass;

	UPROPERTY(EditAnywhere, Category = "Combat|VFX")
	float CasingEjectionForce = 10.f;

	UPROPERTY(EditAnywhere, Category = "Combat|VFX", meta = (ClampMin = "0", ClampMax = "180"))
	FRotator CasingEjectionMaxRotationVariation;


	UPROPERTY(EditAnywhere, Category = "Combat|Audio")
	USoundBase *PickupSound;


	UPROPERTY()
	APawn *FiringInstigator = nullptr;

	UPROPERTY()
	AController *FiringController = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState)
	EWeaponState WeaponState;

	UPROPERTY(ReplicatedUsing = OnRep_MagazineAmmo)
	int32 MagazineAmmo;

	UPROPERTY(ReplicatedUsing = OnRep_ReserveAmmo)
	int32 ReserveAmmo;


	float ZoomInInterpSpeedMultiplier = 1.f;

	FRandomStream SpreadStream;


	virtual void BeginPlay() override;


	virtual void PerformFire(const FVector &Target, float AimValue);


private:

	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult &SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnRep_WeaponState();

	void SwitchWeaponState(EWeaponState State);

	void OnWeaponStateEquipped();

	void OnWeaponStateDropped();

	void EjectAmmoCasing();

	UFUNCTION()
	void OnRep_MagazineAmmo();

	UFUNCTION()
	void OnRep_ReserveAmmo();
};
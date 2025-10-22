// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/WeaponBase.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Weapons/AmmoCasing.h"
#include "Engine/SkeletalMeshSocket.h"


AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// Set up WeaponMesh and its starting collisions.
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Rarity outline effect.
	if (WeaponRarity != ERarity::ER_None)
	{
		WeaponMesh->SetCustomDepthStencilValue(static_cast<int32>(WeaponRarity) - 1);
		WeaponMesh->SetRenderCustomDepth(true);
		WeaponMesh->MarkRenderStateDirty();
	}

	// Set WeaponOverlap collision as disabled for all machines (only the server will enable it later in BeginPlay()).
	WeaponOverlap = CreateDefaultSubobject<USphereComponent>(TEXT("WeaponOverlap"));
	WeaponOverlap->SetupAttachment(RootComponent);
	WeaponOverlap->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	WeaponOverlap->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Set up the weapon's PickupWidget.
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}


#if WITH_EDITOR
void AWeaponBase::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AWeaponBase, WeaponRarity))
	{
		if (WeaponRarity != ERarity::ER_None)
		{
			WeaponMesh->SetCustomDepthStencilValue(static_cast<int32>(WeaponRarity) - 1);
			WeaponMesh->SetRenderCustomDepth(true);
		}
		else
		{
			WeaponMesh->SetRenderCustomDepth(false);
		}

		WeaponMesh->MarkRenderStateDirty();
	}
}
#endif


void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponBase, WeaponState);

	DOREPLIFETIME(AWeaponBase, MagazineAmmo);
	DOREPLIFETIME(AWeaponBase, ReserveAmmo);
}


void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	// Only enable WeaponOverlap and bind its overlap events on the server.
	if (HasAuthority())
	{
		WeaponOverlap->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponOverlap->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

		WeaponOverlap->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnSphereBeginOverlap);
		WeaponOverlap->OnComponentEndOverlap.AddDynamic(this, &AWeaponBase::OnSphereEndOverlap);
	}

	// Disable PickupWidget from displaying by default.
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	// Initialize ammo.
	MagazineAmmo = FMath::Min(StartingAmmo, MagazineSize);
	ReserveAmmo = StartingAmmo - MagazineAmmo;

	// Calculate ZoomInInterpSpeedMultiplier for ZoomInScopes.
	if (AimMode == EWeaponAimMode::EWAM_ZoomIn)
	{
		const AWeaponBase *DefaultWeaponBase = GetDefault<AWeaponBase>();
		float DefaultAimInterpSpeed = DefaultWeaponBase->AimInterpSpeed;
		ZoomInInterpSpeedMultiplier = AimInterpSpeed / DefaultAimInterpSpeed;
	}
}


void AWeaponBase::SetOwner(AActor *NewOwner)
{
	Super::SetOwner(NewOwner);

	FiringInstigator = NewOwner ? Cast<APawn>(NewOwner) : nullptr;
	FiringController = FiringInstigator ? FiringInstigator->GetController() : nullptr;
}


UTexture2D *AWeaponBase::GetKillfeedIcon() const
{
	return KillfeedIcon;
}


void AWeaponBase::OnSphereBeginOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	UCombatComponent *CombatComponent = OtherActor->FindComponentByClass<UCombatComponent>();
	if (CombatComponent)
	{
		CombatComponent->SetOverlappingWeapon(this);
	}
}


void AWeaponBase::OnSphereEndOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp,
	int32 OtherBodyIndex)
{
	UCombatComponent *CombatComponent = OtherActor->FindComponentByClass<UCombatComponent>();
	if (CombatComponent)
	{
		CombatComponent->SetOverlappingWeapon(nullptr);
	}
}


void AWeaponBase::TogglePickupWidget(bool bEnable)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bEnable);
	}
}


void AWeaponBase::SetWeaponState(EWeaponState State)
{
	// This function is meant to be run only on the server.

	// WeaponState is RepNotify.
	WeaponState = State;

	SwitchWeaponState(WeaponState);
}


void AWeaponBase::OnRep_WeaponState()
{
	SwitchWeaponState(WeaponState);
}


void AWeaponBase::SwitchWeaponState(EWeaponState State)
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnWeaponStateEquipped();
		break;

	case EWeaponState::EWS_Dropped:
		OnWeaponStateDropped();
		break;
	}
}


void AWeaponBase::OnWeaponStateEquipped()
{
	if (HasAuthority())
	{
		SetReplicateMovement(false);
	}

	TogglePickupWidget(false);

	WeaponOverlap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(bSupportsMeshPhysics);
	WeaponMesh->SetCollisionEnabled(bSupportsMeshPhysics ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	if (bSupportsMeshPhysics)
	{
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}

	if (WeaponRarity != ERarity::ER_None)
	{
		WeaponMesh->SetRenderCustomDepth(false);
	}
}


void AWeaponBase::OnWeaponStateDropped()
{
	if (HasAuthority())
	{
		WeaponOverlap->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SetReplicateMovement(true);
	}

	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	if (bSupportsMeshPhysics)
	{
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	}

	if (WeaponRarity != ERarity::ER_None)
	{
		WeaponMesh->SetRenderCustomDepth(true);
	}
}


void AWeaponBase::Fire(const FVector &Target, float AimValue)
{
	if (MagazineAmmo <= 0)
	{
		return;
	}

	PerformFire(Target, AimValue);
}


void AWeaponBase::PerformFire(const FVector &Target, float AimValue)
{
	if (FiringAnimation)
	{
		WeaponMesh->PlayAnimation(FiringAnimation, false);
	}

	MagazineAmmo--;
	OnMagazineAmmoChanged.Broadcast(MagazineAmmo);

	EjectAmmoCasing();
}


void AWeaponBase::EjectAmmoCasing()
{
	if (!EjectedCasingClass)
	{
		return;
	}

	// Eject a casing from the ejection port.
	const USkeletalMeshSocket *EjectionPortSocket = WeaponMesh->GetSocketByName(EjectionPortSocketName);
	if (EjectionPortSocket)
	{
		FTransform SocketTransform = EjectionPortSocket->GetSocketTransform(WeaponMesh);

		UWorld *World = GetWorld();
		if (World)
		{
			// Give the casing a random CasingEjectionMaxRotationVariation.
			FRotator CasingRotation = SocketTransform.GetRotation().Rotator() +
				FRotator(FMath::RandRange(-CasingEjectionMaxRotationVariation.Roll, CasingEjectionMaxRotationVariation.Roll),
					FMath::RandRange(-CasingEjectionMaxRotationVariation.Yaw, CasingEjectionMaxRotationVariation.Yaw),
					FMath::RandRange(-CasingEjectionMaxRotationVariation.Pitch, CasingEjectionMaxRotationVariation.Pitch));

			AAmmoCasing *Casing = World->SpawnActor<AAmmoCasing>(EjectedCasingClass, SocketTransform.GetLocation(), CasingRotation);
			Casing->Eject(CasingEjectionForce);
		}
	}
}


void AWeaponBase::Drop(FVector Direction)
{
	if (HasAuthority() && HasNoAmmo())
	{
		Destroy();
	}

	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachmentRules = FDetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachmentRules);
	WeaponMesh->bOwnerNoSee = false;

	SetOwner(nullptr);

	if (HasAuthority() && WeaponMesh)
	{
		Direction.Normalize();
		FVector DropVector = DropForce * Direction;
		WeaponMesh->AddImpulse(DropVector, NAME_None, true);
	}
}


void AWeaponBase::Reload()
{
	if (CanReload())
	{
		int32 AmmoToReload = FMath::Min(MagazineSize - MagazineAmmo, ReserveAmmo);
		MagazineAmmo += AmmoToReload;
		ReserveAmmo -= AmmoToReload;

		OnMagazineAmmoChanged.Broadcast(MagazineAmmo);
		OnReserveAmmoChanged.Broadcast(ReserveAmmo);
	}
}


void AWeaponBase::OnRep_MagazineAmmo()
{
	OnMagazineAmmoChanged.Broadcast(MagazineAmmo);
}


void AWeaponBase::OnRep_ReserveAmmo()
{
	OnReserveAmmoChanged.Broadcast(ReserveAmmo);
}
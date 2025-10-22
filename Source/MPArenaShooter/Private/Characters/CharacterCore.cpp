// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/CharacterCore.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Misc/ConstantsDefinitions.h"
#include "Components/HealthComponent.h"
#include "GameStates/GameStateCore.h"


ACharacterCore::ACharacterCore()
{
	PrimaryActorTick.bCanEverTick = false;

	// Allows the player to always respawn.
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Prevents the camera from being blocked by other players.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	// Set up SpringArm.
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->TargetArmLength = 350.f;
	SpringArm->bUsePawnControlRotation = true;

	// Set up Camera.
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Set variables required for third person movement.
	bUseControllerRotationYaw = false;
	UCharacterMovementComponent *CharacterMovementComponent = GetCharacterMovement();
	CharacterMovementComponent->bOrientRotationToMovement = true;
	CharacterMovementComponent->RotationRate = FRotator(0.f, 0.f, 540.f);
	CharacterMovementComponent->NavAgentProps.bCanCrouch = true;
	CharacterMovementComponent->bCanWalkOffLedgesWhenCrouching = true;

	// Set up OverheadWidget.
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	// Set up CombatComponent.
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	// Set up HealthComponent.
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthComponent->SetIsReplicated(true);

	// Set network performance (66 and 33 are common for shooters).
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}


void ACharacterCore::BeginPlay()
{
	Super::BeginPlay();

	// Disable character's combat functionalities when a round ends.
	UWorld *World = GetWorld();
	if (World)
	{
		AGameStateCore *GameStateCore = World->GetGameState<AGameStateCore>();
		if (GameStateCore)
		{
			GameStateCore->OnRoundEnded.AddDynamic(this, &ACharacterCore::DisableCombat);
		}
	}
}


void ACharacterCore::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context.
	if (APlayerController *PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			int32 Priority = 0;
			Subsystem->AddMappingContext(Controls, Priority);
		}
	}
}


void ACharacterCore::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings.
	UEnhancedInputComponent *EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACharacterCore::Move);
	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACharacterCore::Look);

	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacterCore::StartJump);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacterCore::StopJump);
	EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &ACharacterCore::Interact);
	EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ACharacterCore::ToggleCrouch);

	EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ACharacterCore::StartAiming);
	EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ACharacterCore::StopAiming);
	EnhancedInputComponent->BindAction(FiringAction, ETriggerEvent::Started, this, &ACharacterCore::StartFiring);
	EnhancedInputComponent->BindAction(FiringAction, ETriggerEvent::Completed, this, &ACharacterCore::StopFiring);

	EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Completed, this, &ACharacterCore::Reload);
}


void ACharacterCore::DisableCombat()
{
	if (HealthComponent)
	{
		HealthComponent->SetComponentEnabled(false);
	}

	if (CombatComponent)
	{
		CombatComponent->SetComponentEnabled(false);
		CombatComponent->SetAutoDestroyEquippedWeapon(true);
	}
}


void ACharacterCore::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Falling)
	{
		if (bIsCrouched)
		{
			UnCrouch();
		}
	}
}


void ACharacterCore::Move(const FInputActionValue &Value)
{
	if (CombatComponent)
	{
		FVector2D MovementVector = Value.Get<FVector2D>();
		CombatComponent->ApplyMovement(MovementVector);
	}
}


void ACharacterCore::Look(const FInputActionValue &Value)
{
	if (CombatComponent)
	{
		FVector2D RotationVector = Value.Get<FVector2D>();
		CombatComponent->ApplyRotation(RotationVector);
	}
}


void ACharacterCore::StartJump(const FInputActionValue &Value)
{
	if (CombatComponent)
	{
		CombatComponent->Jump();
	}
}


void ACharacterCore::StopJump(const FInputActionValue &Value)
{
	if (CombatComponent)
	{
		CombatComponent->StopJumping();
	}
}


void ACharacterCore::Interact(const FInputActionValue &Value)
{
	if (CombatComponent)
	{
		CombatComponent->TryPickingUpWeapon();
	}
}


void ACharacterCore::ToggleCrouch(const FInputActionValue &Value)
{
	if (CombatComponent)
	{
		CombatComponent->ToggleCrouch();
	}
}


void ACharacterCore::StartAiming(const FInputActionValue &Value)
{
	if (CombatComponent)
	{
		CombatComponent->ToggleAim(true);
	}
}


void ACharacterCore::StopAiming(const FInputActionValue &Value)
{
	if (CombatComponent)
	{
		CombatComponent->ToggleAim(false);
	}
}


void ACharacterCore::StartFiring(const FInputActionValue &Value)
{
	if (CombatComponent)
	{
		CombatComponent->ToggleFiring(true);
	}
}


void ACharacterCore::StopFiring(const FInputActionValue &Value)
{
	if (CombatComponent)
	{
		CombatComponent->ToggleFiring(false);
	}
}


void ACharacterCore::Reload(const FInputActionValue &Value)
{
	if (CombatComponent)
	{
		CombatComponent->Reload();
	}
}
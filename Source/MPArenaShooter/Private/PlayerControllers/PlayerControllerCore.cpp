// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerControllers/PlayerControllerCore.h"
#include "GameFramework/PlayerState.h"
#include "EnhancedInputComponent.h"
#include "UserInterface/HUDCore.h"
#include "EnhancedInputLibrary.h"
#include "EnhancedInputSubsystems.h"


void APlayerControllerCore::BeginPlay()
{
	Super::BeginPlay();

	AHUD *HUD = GetHUD();
	HUDCore = Cast<AHUDCore>(HUD);
}


void APlayerControllerCore::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	OnPlayerStateReplicated.Broadcast();
}


void APlayerControllerCore::SetupInputComponent()
{
	Super::SetupInputComponent();

	ScrollScoreboardAction->bConsumeInput = false;
	UEnhancedInputLocalPlayerSubsystem *Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	int32 Priority = 1;
	Subsystem->AddMappingContext(UIControls, Priority);

	if (UEnhancedInputComponent *EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInput->BindAction(ToggleScoreboardAction, ETriggerEvent::Started, this, &APlayerControllerCore::ToggleScoreboard);
		EnhancedInput->BindAction(ToggleScoreboardAction, ETriggerEvent::Completed, this, &APlayerControllerCore::ToggleScoreboard);

		EnhancedInput->BindAction(ScrollScoreboardAction, ETriggerEvent::Triggered, this, &APlayerControllerCore::ScrollScoreboard);
	}
}


void APlayerControllerCore::ToggleScoreboard(const FInputActionValue &Value)
{
	bDisplayingScoreboard = Value.Get<bool>();

	ScrollScoreboardAction->bConsumeInput = bDisplayingScoreboard;
	UEnhancedInputLibrary::RequestRebuildControlMappingsUsingContext(UIControls, true);

	if (HUDCore)
	{
		HUDCore->ToggleScoreboard(bDisplayingScoreboard);
	}
}


void APlayerControllerCore::ScrollScoreboard(const FInputActionValue &Value)
{
	if (bDisplayingScoreboard && HUDCore)
	{
		float ScrollAmount = Value.Get<float>();
		HUDCore->ScrollScoreboard(ScrollAmount);
	}
}
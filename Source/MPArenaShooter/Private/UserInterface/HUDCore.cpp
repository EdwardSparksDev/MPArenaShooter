// Fill out your copyright notice in the Description page of Project Settings.

#include "UserInterface/HUDCore.h"
#include "GameStates/GameStateCore.h"
#include "UserInterface/PreRoundWidget.h"
#include "UserInterface/ScoreboardWidget.h"
#include "UserInterface/CombatWidget.h"
#include "UserInterface/PostRoundWidget.h"
#include "GameFramework/PlayerState.h"
#include "PlayerControllers/PlayerControllerCore.h"
#include "Components/CombatComponent.h"
#include "Weapons/WeaponBase.h"
#include "UserInterface/ZoomInScopeWidget.h"


AHUDCore::AHUDCore()
{
	PrimaryActorTick.bCanEverTick = false;
}


void AHUDCore::BeginPlay()
{
	Super::BeginPlay();

	BindToGameState();
	BindToPlayerController();
}


void AHUDCore::BindToGameState()
{
	UWorld *World = GetWorld();
	if (!World)
	{
		return;
	}

	AGameStateCore *GameStateCore = World->GetGameState<AGameStateCore>();
	if (!GameStateCore->HasMatchStarted())
	{
		GameStateCore->OnRoundStarted.AddDynamic(this, &ThisClass::OnRoundStarted);
		GameStateCore->OnRoundEnded.AddDynamic(this, &ThisClass::OnRoundEnded);

		OnPreRoundStarted();
	}
	else if (GameStateCore->IsMatchInProgress())
	{
		GameStateCore->OnRoundEnded.AddDynamic(this, &ThisClass::OnRoundEnded);

		OnRoundStarted();
	}
	else
	{
		OnRoundEnded();
	}
}


void AHUDCore::OnPreRoundStarted()
{
	if (PreRoundWidgetClass)
	{
		PreRoundWidget = CreateWidget<UPreRoundWidget>(GetOwningPlayerController(), PreRoundWidgetClass);

		if (PreRoundWidget)
		{
			PreRoundWidget->AddToViewport();
		}
	}
}


void AHUDCore::OnRoundStarted()
{
	if (PreRoundWidget)
	{
		PreRoundWidget->RemoveFromParent();
	}

	if (CombatWidgetClass)
	{
		CombatWidget = CreateWidget<UCombatWidget>(GetOwningPlayerController(), CombatWidgetClass);

		if (CombatWidget)
		{
			CombatWidget->AddToViewport();

			// Set CombatDisplayWidget's components initial visibility.
			CombatWidget->SetCrosshairVisibility(bDisplayCrosshair ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
			CombatWidget->SetHealthBarVisibility(bDisplayHealthBar ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
			CombatWidget->SetPlayerScoreVisibility(bDisplayPlayerScore ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
			CombatWidget->SetWeaponInfoVisibility(bDisplayWeaponInfo ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
			CombatWidget->SetRoundTimerVisibility(bDisplayMatchTimer ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		}
	}
}


void AHUDCore::OnRoundEnded()
{
	if (CombatWidget)
	{
		CombatWidget->RemoveFromParent();
	}

	if (PostRoundWidgetClass)
	{
		PostRoundWidget = CreateWidget<UPostRoundWidget>(GetOwningPlayerController(), PostRoundWidgetClass);

		if (PostRoundWidget)
		{
			PostRoundWidget->AddToViewport();
		}
	}
}


void AHUDCore::BindToPlayerController()
{
	APlayerControllerCore *PlayerControllerCore = Cast<APlayerControllerCore>(GetOwningPlayerController());
	PlayerControllerCore->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnControllerPossessedPawnChanged);

	APawn *LocalPawn = PlayerControllerCore->GetPawn();
	TogglePawnComponentsBindings(LocalPawn, true);
}


void AHUDCore::OnControllerPossessedPawnChanged(APawn *OldPawn, APawn *NewPawn)
{
	TogglePawnComponentsBindings(OldPawn, false);
	TogglePawnComponentsBindings(NewPawn, true);
}


void AHUDCore::TogglePawnComponentsBindings(APawn *Pawn, bool bEnable)
{
	if (!Pawn)
	{
		return;
	}

	UCombatComponent *CombatComp = Pawn->FindComponentByClass<UCombatComponent>();
	if (!CombatComp)
	{
		return;
	}

	if (bEnable)
	{
		CombatComp->OnAimStateChanged.AddDynamic(this, &ThisClass::OnAimStateChanged);
		CombatComp->OnEquippedWeaponChanged.AddDynamic(this, &ThisClass::OnEquippedWeaponChanged);
	}
	else
	{
		CombatComp->OnAimStateChanged.RemoveDynamic(this, &ThisClass::OnAimStateChanged);
		CombatComp->OnEquippedWeaponChanged.RemoveDynamic(this, &ThisClass::OnEquippedWeaponChanged);
	}
}


void AHUDCore::OnEquippedWeaponChanged(UCombatComponent *CombatComp)
{
	if (!CombatComp)
	{
		return;
	}

	AWeaponBase *EquippedWeapon = CombatComp->GetEquippedWeapon();

	if (IsValid(EquippedWeapon))
	{
		// Check if the active ZoomInScope class needs to be updated.
		TSubclassOf<UZoomInScopeWidget> ZoomInScopeWidgetClass = EquippedWeapon->GetWeaponZoomInScopeWidget();
		if (ZoomInScopeWidgetClass && ActiveZoomInScopeWidgetClass != ZoomInScopeWidgetClass)
		{
			EAimState AimState = CombatComp->GetAimState();
			float AimValue = CombatComp->GetAimValue();
			SwitchZoomInScope(ZoomInScopeWidgetClass, AimValue, AimState);
		}

		EWeaponAimMode NewAimMode = EquippedWeapon->GetWeaponAimMode();
		bCanDisplayZoomInScope = (NewAimMode == EWeaponAimMode::EWAM_ZoomIn);
		if (bCanDisplayZoomInScope)
		{
			LastZoomInInterpSpeedMultiplier = EquippedWeapon->GetZoomInInterpSpeedMultiplier();
		}
	}
	else
	{
		bCanDisplayZoomInScope = false;
	}

	// Update ActiveZoomInScopeWidget's visibility.
	if (ActiveZoomInScopeWidget)
	{
		ESlateVisibility ZoomInScopeVisibilty = ActiveZoomInScopeWidget->GetVisibility();
		if (bCanDisplayZoomInScope && ZoomInScopeVisibilty == ESlateVisibility::Hidden)
		{
			bool bIsAiming = CombatComp->IsAiming();
			if (bIsAiming)
			{
				ActiveZoomInScopeWidget->ToggleZoom(true, LastZoomInInterpSpeedMultiplier);
			}
		}
		else if (!bCanDisplayZoomInScope && ZoomInScopeVisibilty == ESlateVisibility::Visible)
		{
			ActiveZoomInScopeWidget->ToggleZoom(false, LastZoomInInterpSpeedMultiplier);
		}
	}
}


void AHUDCore::SwitchZoomInScope(TSubclassOf<UZoomInScopeWidget> ZoomInScopeWidgetClass, float AimValue, EAimState AimState)
{
	if (ActiveZoomInScopeWidget)
	{
		ActiveZoomInScopeWidget->RemoveFromParent();
	}

	APlayerController *LocalPlayerController = GetOwningPlayerController();
	ActiveZoomInScopeWidget = CreateWidget<UZoomInScopeWidget>(LocalPlayerController, ZoomInScopeWidgetClass);
	if (ActiveZoomInScopeWidget)
	{
		ActiveZoomInScopeWidgetClass = ZoomInScopeWidgetClass;

		bool bZoomIn = (AimState == EAimState::EAS_ZoomInStarted);
		ActiveZoomInScopeWidget->InitializeScope(bZoomIn, AimValue);
	}
}


void AHUDCore::OnAimStateChanged(EAimState AimState)
{
	if (!bCanDisplayZoomInScope || !ActiveZoomInScopeWidget)
	{
		return;
	}

	if (AimState == EAimState::EAS_ZoomInStarted)
	{
		ActiveZoomInScopeWidget->ToggleZoom(true, LastZoomInInterpSpeedMultiplier);
	}
	else if (AimState == EAimState::EAS_ZoomOutStarted)
	{
		ActiveZoomInScopeWidget->ToggleZoom(false, LastZoomInInterpSpeedMultiplier);
	}
}


void AHUDCore::ToggleScoreboard(bool bEnable)
{
	if (bEnable)
	{
		APlayerController *LocalPlayerController = GetOwningPlayerController();
		APlayerState *LocalPlayerState = LocalPlayerController->GetPlayerState<APlayerState>();
		if (!LocalPlayerState)
		{
			return;
		}

		if (!ScoreboardWidget)
		{
			ScoreboardWidget = CreateWidget<UScoreboardWidget>(LocalPlayerController, ScoreboardWidgetClass);
			if (!ScoreboardWidget)
			{
				return;
			}

			ScoreboardWidget->AddToViewport(ScoreboardZOrder);
		}

		ScoreboardWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else if (ScoreboardWidget)
	{
		ScoreboardWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}


void AHUDCore::ScrollScoreboard(float Amount)
{
	if (ScoreboardWidget)
	{
		ScoreboardWidget->ScrollScoreboardWithoutFocus(Amount);
	}
}
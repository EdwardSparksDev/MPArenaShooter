// Fill out your copyright notice in the Description page of Project Settings.

#include "UserInterface/CombatWidget.h"
#include "GameStates/GameStateCore.h"
#include "PlayerControllers/PlayerControllerCore.h"
#include "Components/CombatComponent.h"
#include "Components/HealthComponent.h"
#include "PlayerStates/PlayerStateCore.h"
#include "UserInterface/CrosshairWidget.h"
#include "Weapons/WeaponBase.h"
#include "UserInterface/HealthBarWidget.h"
#include "UserInterface/RoundScoreWidget.h"
#include "UserInterface/WeaponInfoWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"


void UCombatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToGameState();
	BindToPlayerController();
}


void UCombatWidget::BindToGameState()
{
	AGameStateCore *GameStateCore = GetWorld()->GetGameState<AGameStateCore>();
	GameStateCore->OnRoundTimerUpdated.AddDynamic(this, &ThisClass::OnRoundTimerUpdated);
	GameStateCore->OnFirstBlood.AddDynamic(this, &ThisClass::OnFirstBlood);

	int32 RoundTime = GameStateCore->GetRoundTime();
	if (RoundTime > 0)
	{
		OnRoundTimerUpdated(RoundTime);
	}
}


void UCombatWidget::BindToPlayerController()
{
	APlayerControllerCore *PlayerControllerCore = Cast<APlayerControllerCore>(GetOwningPlayer());
	PlayerControllerCore->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnControllerPossessedPawnChanged);

	APawn *LocalPawn = PlayerControllerCore->GetPawn();
	TogglePawnComponentsBindings(LocalPawn, true);
}


void UCombatWidget::OnControllerPossessedPawnChanged(APawn *OldPawn, APawn *NewPawn)
{
	TogglePawnComponentsBindings(OldPawn, false);
	TogglePawnComponentsBindings(NewPawn, true);
}


void UCombatWidget::TogglePawnComponentsBindings(APawn *Pawn, bool bEnable)
{
	if (!Pawn)
	{
		return;
	}

	UCombatComponent *CombatComp = Pawn->FindComponentByClass<UCombatComponent>();
	UHealthComponent *HealthComp = Pawn->FindComponentByClass<UHealthComponent>();
	if (!CombatComp || !HealthComp)
	{
		return;
	}

	if (bEnable)
	{
		CombatComp->OnAimValueChanged.AddDynamic(this, &ThisClass::OnAimValueChanged);
		CombatComp->OnAimStateChanged.AddDynamic(this, &ThisClass::OnAimStateChanged);

		CombatComp->OnEquippedWeaponChanged.AddDynamic(this, &ThisClass::OnEquippedWeaponChanged);
		CombatComp->OnCrosshairSpreadChanged.AddDynamic(this, &ThisClass::OnCrosshairSpreadChanged);
		CombatComp->OnCrosshairColorChanged.AddDynamic(this, &ThisClass::OnCrosshairColorChanged);

		HealthComp->OnHealthChanged.AddDynamic(this, &ThisClass::OnHealthChanged);

		LoadInitialValues(CombatComp, HealthComp);
	}
	else
	{
		CombatComp->OnAimValueChanged.RemoveDynamic(this, &ThisClass::OnAimValueChanged);
		CombatComp->OnAimStateChanged.RemoveDynamic(this, &ThisClass::OnAimStateChanged);

		CombatComp->OnEquippedWeaponChanged.RemoveDynamic(this, &ThisClass::OnEquippedWeaponChanged);
		CombatComp->OnCrosshairSpreadChanged.RemoveDynamic(this, &ThisClass::OnCrosshairSpreadChanged);
		CombatComp->OnCrosshairColorChanged.RemoveDynamic(this, &ThisClass::OnCrosshairColorChanged);

		HealthComp->OnHealthChanged.RemoveDynamic(this, &ThisClass::OnHealthChanged);
	}
}


void UCombatWidget::LoadInitialValues(UCombatComponent *CombatComp, UHealthComponent *HealthComp)
{
	// Load weapon values.
	AWeaponBase *EquippedWeapon = CombatComp->GetEquippedWeapon();
	int32 CurrentMagazineAmmo = 0;
	int32 CurrentReserveAmmo = 0;
	if (IsValid(EquippedWeapon))
	{
		CurrentMagazineAmmo = EquippedWeapon->GetMagazineAmmo();
		CurrentReserveAmmo = EquippedWeapon->GetReserveAmmo();
	}

	OnMagazineAmmoChanged(CurrentMagazineAmmo);
	OnReserveAmmoChanged(CurrentReserveAmmo);

	// Load health values.
	int32 CurrentHealth = HealthComp->GetHealth();
	int32 MaxHealth = HealthComp->GetMaxHealth();
	OnHealthChanged(CurrentHealth, MaxHealth);
}


void UCombatWidget::SetCrosshairVisibility(ESlateVisibility NewVisibility)
{
	CrosshairWidget->SetVisibility(NewVisibility);
}


void UCombatWidget::OnAimValueChanged(float AimValue)
{
	FVector2D CurrentSpreadRange = FMath::Lerp(SpreadRange, AimSpreadRange, AimValue);
	CrosshairWidget->UpdateSpreadRange(CurrentSpreadRange);
}


void UCombatWidget::OnAimStateChanged(EAimState AimState)
{
	if (AimState == EAimState::EAS_ZoomInStarted && bHideReticleOnZoom)
	{
		CrosshairWidget->ToggleReticleVisibility(ESlateVisibility::Hidden);
	}
	else if (AimState == EAimState::EAS_ZoomOutCompleted)
	{
		CrosshairWidget->ToggleReticleVisibility(ESlateVisibility::Visible);
	}
}


void UCombatWidget::OnEquippedWeaponChanged(UCombatComponent *CombatComp)
{
	if (!CombatComp)
	{
		return;
	}

	if (IsValid(LastEquippedWeapon))
	{
		LastEquippedWeapon->OnMagazineAmmoChanged.RemoveDynamic(this, &ThisClass::OnMagazineAmmoChanged);
		LastEquippedWeapon->OnReserveAmmoChanged.RemoveDynamic(this, &ThisClass::OnReserveAmmoChanged);
	}

	AWeaponBase *EquippedWeapon = CombatComp->GetEquippedWeapon();
	FCrosshairTextures Textures;
	int32 CurrentMagazineAmmo = 0;
	int32 CurrentReserveAmmo = 0;

	float AimValue = CombatComp->GetAimValue();

	if (IsValid(EquippedWeapon))
	{
		Textures = EquippedWeapon->GetCrosshairTextures();
		SpreadRange = EquippedWeapon->GetCrosshairSpreadRange();
		AimSpreadRange = EquippedWeapon->GetCrosshairAimSpreadRange();

		EWeaponAimMode WeaponAimMode = EquippedWeapon->GetWeaponAimMode();
		bool bIsZoomInWeapon = (WeaponAimMode == EWeaponAimMode::EWAM_ZoomIn);
		bool bIsAiming = CombatComp->IsAiming();
		if (bHideReticleOnZoom && !bIsZoomInWeapon)
		{
			bHideReticleOnZoom = false;
			CrosshairWidget->ToggleReticleVisibility(ESlateVisibility::Visible);
		}
		else if (!bHideReticleOnZoom && bIsZoomInWeapon)
		{
			bHideReticleOnZoom = true;
			if (bIsAiming)
			{
				CrosshairWidget->ToggleReticleVisibility(ESlateVisibility::Hidden);
			}
		}

		CurrentMagazineAmmo = EquippedWeapon->GetMagazineAmmo();
		EquippedWeapon->OnMagazineAmmoChanged.AddDynamic(this, &ThisClass::OnMagazineAmmoChanged);
		CurrentReserveAmmo = EquippedWeapon->GetReserveAmmo();
		EquippedWeapon->OnReserveAmmoChanged.AddDynamic(this, &ThisClass::OnReserveAmmoChanged);
	}
	else
	{
		Textures = CombatComp->GetDefaultCrosshairTextures();
		SpreadRange = FVector2D::ZeroVector;
		AimSpreadRange = FVector2D::ZeroVector;
	}

	OnAimValueChanged(AimValue);

	CrosshairWidget->ApplyTextures(Textures);
	OnMagazineAmmoChanged(CurrentMagazineAmmo);
	OnReserveAmmoChanged(CurrentReserveAmmo);

	LastEquippedWeapon = EquippedWeapon;
}


void UCombatWidget::OnCrosshairSpreadChanged(float Value)
{
	CrosshairWidget->UpdateSpread(Value);
}


void UCombatWidget::OnCrosshairColorChanged(FLinearColor Color)
{
	CrosshairWidget->SetCrosshairColor(Color);
}


void UCombatWidget::SetHealthBarVisibility(ESlateVisibility NewVisibility)
{
	HealthBarWidget->SetVisibility(NewVisibility);
}


void UCombatWidget::OnHealthChanged(float Health, float MaxHealth)
{
	HealthBarWidget->UpdateValue(Health, MaxHealth);
}


void UCombatWidget::SetPlayerScoreVisibility(ESlateVisibility NewVisibility)
{
	RoundScoreWidget->SetVisibility(NewVisibility);
}


void UCombatWidget::SetWeaponInfoVisibility(ESlateVisibility NewVisibility)
{
	WeaponInfoWidget->SetVisibility(NewVisibility);
}


void UCombatWidget::OnMagazineAmmoChanged(int32 MagazineAmmo)
{
	WeaponInfoWidget->UpdateMagazineAmmo(MagazineAmmo);
}


void UCombatWidget::OnReserveAmmoChanged(int32 ReserveAmmo)
{
	WeaponInfoWidget->UpdateReserveAmmo(ReserveAmmo);
}


void UCombatWidget::SetRoundTimerVisibility(ESlateVisibility NewVisibility)
{
	RoundTimer_TXT->SetVisibility(NewVisibility);
}


void UCombatWidget::OnRoundTimerUpdated(int32 Time)
{
	int32 Minutes = Time / 60;
	int32 Seconds = Time - Minutes * 60;
	FString TimerString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
	RoundTimer_TXT->SetText(FText::FromString(TimerString));

	if (Time <= CriticalTimeThreshold)
	{
		FSlateColor CurrentTimerColor = RoundTimer_TXT->GetColorAndOpacity();
		if (CurrentTimerColor != CriticalTimeColor)
		{
			RoundTimer_TXT->SetColorAndOpacity(CriticalTimeColor);
		}

		if (Time > 0)
		{
			float TimeDilation = UGameplayStatics::GetGlobalTimeDilation(this);
			PlayAnimation(Anim_RoundTimer, 0.f, 1, EUMGSequencePlayMode::Forward, TimeDilation, true);

			if (CriticalTimeSound)
			{
				UGameplayStatics::PlaySound2D(this, CriticalTimeSound);
			}
		}
	}
}


void UCombatWidget::OnFirstBlood(APlayerStateCore *PlayerStateCore)
{
	if (!IsValid(PlayerStateCore))
	{
		return;
	}

	FString PlayerName = PlayerStateCore->GetPlayerName();
	FirstBloodPlayerName_TXT->SetText(FText::FromString(PlayerName));

	APlayerController *LocalPlayerController = GetOwningPlayer();
	APlayerStateCore *LocalPlayerStateCore = LocalPlayerController->GetPlayerState<APlayerStateCore>();
	if (LocalPlayerStateCore == PlayerStateCore)
	{
		FirstBloodPlayerName_TXT->SetColorAndOpacity(LocalPlayerColor);
	}
	else
	{
		FirstBloodPlayerName_TXT->SetColorAndOpacity(EnemyPlayerColor);
	}

	PlayAnimation(Anim_FirstBlood);
	if (FirstBloodSound)
	{
		UGameplayStatics::PlaySound2D(this, FirstBloodSound);
	}
}
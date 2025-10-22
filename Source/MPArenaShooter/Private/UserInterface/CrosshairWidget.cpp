// Fill out your copyright notice in the Description page of Project Settings.

#include "UserInterface/CrosshairWidget.h"
#include "GameStates/GameStateCore.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "PlayerStates/PlayerStateCore.h"
#include "Kismet/GameplayStatics.h"


void UCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToGameState();

	SetCrosshairComponentTexture2D(CrosshairCenter_IMG, nullptr);
	SetCrosshairComponentTexture2D(CrosshairTop_IMG, nullptr);
	SetCrosshairComponentTexture2D(CrosshairBottom_IMG, nullptr);
	SetCrosshairComponentTexture2D(CrosshairRight_IMG, nullptr);
	SetCrosshairComponentTexture2D(CrosshairLeft_IMG, nullptr);
}


void UCrosshairWidget::BindToGameState()
{
	UWorld *World = GetWorld();
	if (!World)
	{
		return;
	}

	AGameStateCore *GameStateCore = World->GetGameState<AGameStateCore>();
	GameStateCore->OnKillfeedUpdated.AddDynamic(this, &ThisClass::OnKillfeedUpdated);
}


void UCrosshairWidget::ToggleReticleVisibility(ESlateVisibility NewVisibility)
{
	if (bReticleHidden && NewVisibility == ESlateVisibility::Visible)
	{
		bReticleHidden = false;
	}
	else if (!bReticleHidden && NewVisibility == ESlateVisibility::Hidden)
	{
		bReticleHidden = true;
	}
	else
	{
		return;
	}

	SetCrosshairComponentVisibility(CrosshairCenter_IMG, NewVisibility);
	SetCrosshairComponentVisibility(CrosshairTop_IMG, NewVisibility);
	SetCrosshairComponentVisibility(CrosshairBottom_IMG, NewVisibility);
	SetCrosshairComponentVisibility(CrosshairRight_IMG, NewVisibility);
	SetCrosshairComponentVisibility(CrosshairLeft_IMG, NewVisibility);
}


void UCrosshairWidget::SetCrosshairComponentVisibility(UImage *Image, ESlateVisibility NewVisibility)
{
	if (!Image)
	{
		return;
	}

	bool bHasValidTexture = Image->GetBrush().GetResourceObject() != nullptr;
	ESlateVisibility UpdatedVisibility = bHasValidTexture ? NewVisibility : ESlateVisibility::Hidden;
	Image->SetVisibility(UpdatedVisibility);
}


void UCrosshairWidget::ApplyTextures(const FCrosshairTextures &Textures)
{
	if (CurrentTextures != Textures)
	{
		CurrentTextures = Textures;

		// Apply crosshair textures.
		SetCrosshairComponentTexture2D(CrosshairCenter_IMG, Textures.CrosshairCenter);
		SetCrosshairComponentTexture2D(CrosshairTop_IMG, Textures.CrosshairTop);
		SetCrosshairComponentTexture2D(CrosshairBottom_IMG, Textures.CrosshairBottom);
		SetCrosshairComponentTexture2D(CrosshairRight_IMG, Textures.CrosshairRight);
		SetCrosshairComponentTexture2D(CrosshairLeft_IMG, Textures.CrosshairLeft);
	}
}


void UCrosshairWidget::SetCrosshairComponentTexture2D(UImage *Image, UTexture2D *Texture)
{
	if (!Image)
	{
		return;
	}

	ESlateVisibility NewVisibility = Texture ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
	Image->SetVisibility(NewVisibility);

	Image->SetBrushFromTexture(Texture, true);
}


void UCrosshairWidget::UpdateSpreadRange(FVector2D SpreadRange)
{
	if (CurrentSpreadRange != SpreadRange)
	{
		CurrentSpreadRange = SpreadRange;

		UpdateSpread(CurrentSpread, true);
	}
}


void UCrosshairWidget::UpdateSpread(float Spread, bool bForceUpdate)
{
	if (bForceUpdate || CurrentSpread != Spread)
	{
		CurrentSpread = Spread;

		TranslateCrosshairComponentToLocation(CrosshairTop_IMG, FVector2D(0.f, -CurrentSpreadRange.X - Spread * CurrentSpreadRange.Y));
		TranslateCrosshairComponentToLocation(CrosshairBottom_IMG, FVector2D(0.f, CurrentSpreadRange.X + Spread * CurrentSpreadRange.Y));
		TranslateCrosshairComponentToLocation(CrosshairRight_IMG, FVector2D(CurrentSpreadRange.X + Spread * CurrentSpreadRange.Y, 0.f));
		TranslateCrosshairComponentToLocation(CrosshairLeft_IMG, FVector2D(-CurrentSpreadRange.X - Spread * CurrentSpreadRange.Y, 0.f));
	}
}


void UCrosshairWidget::TranslateCrosshairComponentToLocation(UImage *Image, const FVector2D &Location)
{
	if (Image)
	{
		Image->SetRenderTranslation(Location);
	}
}


void UCrosshairWidget::SetCrosshairColor(FLinearColor Color)
{
	CrosshairCenter_IMG->SetColorAndOpacity(Color);
	CrosshairTop_IMG->SetColorAndOpacity(Color);
	CrosshairBottom_IMG->SetColorAndOpacity(Color);
	CrosshairRight_IMG->SetColorAndOpacity(Color);
	CrosshairLeft_IMG->SetColorAndOpacity(Color);
}


void UCrosshairWidget::OnKillfeedUpdated(APlayerStateCore *AttackerPlayerStateCore, APlayerStateCore *EliminatedPlayerStateCore,
	AActor *DamageCauser)
{
	if (!IsValid(AttackerPlayerStateCore))
	{
		return;
	}

	APlayerController *LocalPlayerController = GetOwningPlayer();
	APlayerStateCore *LocalPlayerStateCore = LocalPlayerController->GetPlayerState<APlayerStateCore>();
	if (LocalPlayerStateCore == AttackerPlayerStateCore && AttackerPlayerStateCore != EliminatedPlayerStateCore)
	{
		if (KillMarkerSound)
		{
			UGameplayStatics::PlaySound2D(this, KillMarkerSound);
		}

		PlayAnimation(Anim_KillMarker);
	}
}
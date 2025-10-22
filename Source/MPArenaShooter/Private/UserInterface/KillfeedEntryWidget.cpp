// Fill out your copyright notice in the Description page of Project Settings.

#include "UserInterface/KillfeedEntryWidget.h"
#include "PlayerStates/PlayerStateCore.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Interfaces/KillfeedInterface.h"


void UKillfeedEntryWidget::InitializeEntry(APlayerStateCore *AttackerPlayerStateCore, APlayerStateCore *EliminatedPlayerStateCore,
	AActor *DamageCauser, float DisplayDuration, float AttackerSlotMaxDesiredWidth)
{
	if (bIsActive)
	{
		return;
	}

	bIsActive = true;
	ActiveDisplayDuration = DisplayDuration;

	APlayerController *LocalPlayerController = GetOwningPlayer();
	APlayerStateCore *LocalPlayerStateCore = LocalPlayerController->GetPlayerState<APlayerStateCore>();

	// If the player committed suicide, only show the eliminated player's name and not the attacker's name (since it's the same).
	if (AttackerPlayerStateCore != EliminatedPlayerStateCore)
	{
		SetPlayerText(AttackerPlayerName_TXT, AttackerPlayerStateCore, LocalPlayerStateCore);
		AttackerSlot_SBX->SetMaxDesiredWidth(AttackerSlotMaxDesiredWidth);
		AttackerPlayerName_TXT->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		AttackerPlayerName_TXT->SetVisibility(ESlateVisibility::Collapsed);
	}

	SetPlayerText(EliminatedPlayerName_TXT, EliminatedPlayerStateCore, LocalPlayerStateCore);
	SetKillIcon(DamageCauser);

	bFadingIn = true;
	StartFadeIn();
}


void UKillfeedEntryWidget::SetPlayerText(UTextBlock *PlayerTextBlock, APlayerStateCore *PlayerStateCore, APlayerStateCore *LocalPlayerStateCore)
{
	if (!IsValid(PlayerStateCore))
	{
		PlayerTextBlock->SetText(UnknownPlayerName);
		PlayerTextBlock->SetColorAndOpacity(EnemyPlayerColor);
		return;
	}

	FString PlayerString = PlayerStateCore->GetPlayerName();
	PlayerTextBlock->SetText(FText::FromString(PlayerString));

	if (IsValid(LocalPlayerStateCore))
	{
		if (LocalPlayerStateCore == PlayerStateCore)
		{
			PlayerTextBlock->SetColorAndOpacity(LocalPlayerColor);
		}
		else
		{
			PlayerTextBlock->SetColorAndOpacity(EnemyPlayerColor);
		}
	}
}


void UKillfeedEntryWidget::SetKillIcon(AActor *DamageCauser)
{
	UTexture2D *KillIcon = nullptr;
	if (IsValid(DamageCauser))
	{
		IKillfeedInterface *Interface = Cast<IKillfeedInterface>(DamageCauser);
		if (Interface)
		{
			KillIcon = Interface->GetKillfeedIcon();
		}
	}

	KillIcon_IMG->SetBrushFromTexture(KillIcon, true);
}


void UKillfeedEntryWidget::StartFadeIn()
{
	OnFadeInCompleted.BindDynamic(this, &ThisClass::StartActiveLifetime);
	BindToAnimationFinished(Anim_FadeIn, OnFadeInCompleted);

	PlayAnimation(Anim_FadeIn);
}


void UKillfeedEntryWidget::StartActiveLifetime()
{
	bFadingIn = false;
	UnbindFromAnimationFinished(Anim_FadeIn, OnFadeInCompleted);

	if (bFadingOut)
	{
		StartFadeOut();
	}
	else
	{
		UWorld *World = GetWorld();
		if (!World)
		{
			return;
		}

		World->GetTimerManager().SetTimer(TimerHandle_Despawn, this, &ThisClass::DespawnEntry, ActiveDisplayDuration);
	}
}


void UKillfeedEntryWidget::StartFadeOut()
{
	OnFadeOutCompleted.BindDynamic(this, &ThisClass::OnEntryDespawned);
	BindToAnimationFinished(Anim_FadeIn, OnFadeOutCompleted);

	PlayAnimationReverse(Anim_FadeIn);
}


void UKillfeedEntryWidget::DespawnEntry()
{
	if (bFadingOut)
	{
		return;
	}

	bFadingOut = true;

	if (TimerHandle_Despawn.IsValid())
	{
		UWorld *World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(TimerHandle_Despawn);
			TimerHandle_Despawn.Invalidate();
		}
	}

	if (!bFadingIn)
	{
		StartFadeOut();
	}
}


void UKillfeedEntryWidget::OnEntryDespawned()
{
	bFadingOut = false;
	UnbindFromAnimationFinished(Anim_FadeIn, OnFadeOutCompleted);

	bIsActive = false;
	OnKillfeedEntryDespawned.Broadcast(this);
}
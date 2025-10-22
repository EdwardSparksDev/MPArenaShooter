// Fill out your copyright notice in the Description page of Project Settings.

#include "UserInterface/KillfeedWidget.h"
#include "GameStates/GameStateCore.h"
#include "UserInterface/KillfeedEntryWidget.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"


void UKillfeedWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToGameState();
}


void UKillfeedWidget::BindToGameState()
{
	UWorld *World = GetWorld();
	if (!World)
	{
		return;
	}

	AGameStateCore *GameStateCore = World->GetGameState<AGameStateCore>();
	GameStateCore->OnKillfeedUpdated.AddDynamic(this, &ThisClass::OnKillfeedUpdated);
}


void UKillfeedWidget::OnKillfeedUpdated(APlayerStateCore *AttackerPlayerStateCore, APlayerStateCore *EliminatedPlayerStateCore,
	AActor *DamageCauser)
{
	if (!AttackerPlayerStateCore || !EliminatedPlayerStateCore)
	{
		return;
	}

	// Get a new KillfeedEntry either by the entries pool or by creating a new one.
	UKillfeedEntryWidget *KillfeedEntry = nullptr;
	if (!PooledEntries.IsEmpty())
	{
		KillfeedEntry = PooledEntries[0];
		PooledEntries.RemoveAt(0, EAllowShrinking::No);
	}
	else
	{
		APlayerController *LocalPlayerController = GetOwningPlayer();
		KillfeedEntry = CreateWidget<UKillfeedEntryWidget>(LocalPlayerController, KillfeedEntryWidgetClass);
		KillfeedEntry->OnKillfeedEntryDespawned.AddDynamic(this, &ThisClass::OnKillfeedEntryDespawned);
	}

	// Initialize the KillfeedEntry and add it on screen.
	KillfeedEntry->InitializeEntry(AttackerPlayerStateCore, EliminatedPlayerStateCore, DamageCauser, ActiveEntriesDisplayDuration,
		AttackerSlotMaxDesiredWidth);

	UPanelSlot *PanelSlot = KillfeedPanel_OVR->AddChild(KillfeedEntry);
	UOverlaySlot *OverlaySlot = Cast<UOverlaySlot>(PanelSlot);
	OverlaySlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);

	// Start despawning entries if there's more than MaxDisplayedActiveEntries.
	DisplayedActiveEntries.Add(KillfeedEntry);
	if (DisplayedActiveEntries.Num() > MaxDisplayedActiveEntries)
	{
		UKillfeedEntryWidget *EntryToDespawn = DisplayedActiveEntries[0];
		DisplayedActiveEntries.RemoveAt(0, EAllowShrinking::No);
		EntryToDespawn->DespawnEntry();
	}

	PanKillfeed();
}


void UKillfeedWidget::OnKillfeedEntryDespawned(UKillfeedEntryWidget *DespawnedEntry)
{
	KillfeedPanel_OVR->RemoveChild(DespawnedEntry);

	DisplayedActiveEntries.Remove(DespawnedEntry);
	PooledEntries.Add(DespawnedEntry);
}


void UKillfeedWidget::PanKillfeed()
{
	float YSpacing = KillfeedPanel_OVR->GetCachedGeometry().GetLocalSize().Y;
	TArray<UWidget *> DisplayedKillfeedEntries = KillfeedPanel_OVR->GetAllChildren();

	int32 TotalDisplayedActiveEntries = DisplayedKillfeedEntries.Num();
	float MaxYSpacing = YSpacing * (TotalDisplayedActiveEntries - 1);
	for (int Index = 0; Index < TotalDisplayedActiveEntries; Index++)
	{
		FWidgetTransform EntryTransform = DisplayedKillfeedEntries[Index]->GetRenderTransform();
		EntryTransform.Translation = FVector2D(EntryTransform.Translation.X, YSpacing * Index - MaxYSpacing);
		DisplayedKillfeedEntries[Index]->SetRenderTransform(EntryTransform);
	}
}
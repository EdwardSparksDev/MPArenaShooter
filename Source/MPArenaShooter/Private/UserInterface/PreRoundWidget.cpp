// Fill out your copyright notice in the Description page of Project Settings.

#include "UserInterface/PreRoundWidget.h"
#include "GameStates/GameStateCore.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "Kismet/GameplayStatics.h"


void UPreRoundWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToGameState();
}


void UPreRoundWidget::BindToGameState()
{
	AGameStateCore *GameStateCore = GetWorld()->GetGameState<AGameStateCore>();
	GameStateCore->OnPreRoundTimerUpdated.AddDynamic(this, &ThisClass::OnPreRoundTimerUpdated);

	int32 PreRoundTime = GameStateCore->GetPreRoundTime();
	OnPreRoundTimerUpdated(PreRoundTime);
}


void UPreRoundWidget::OnPreRoundTimerUpdated(int32 Time)
{
	FString TimerString = FString::Printf(TEXT("%d"), Time);
	PreRoundTimer_TXT->SetText(FText::FromString(TimerString));

	if (Time > 0)
	{
		float TimeDilation = UGameplayStatics::GetGlobalTimeDilation(this);
		PlayAnimation(Anim_PreRoundTimer, 0.f, 1, EUMGSequencePlayMode::Forward, TimeDilation);

		if (Time <= CriticalTimeThreshold && CriticalTimeSound)
		{
			UGameplayStatics::PlaySound2D(this, CriticalTimeSound);
		}
	}
}
// Fill out your copyright notice in the Description page of Project Settings.

#include "UserInterface/HealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"


void UHealthBarWidget::UpdateValue(float Health, float MaxHealth)
{
	if (MaxHealth == 0.f)
	{
		return;
	}

	float Percentage = FMath::Clamp(Health / MaxHealth, 0.f, 1.f);
	HealthBarFill_PRG->SetPercent(Percentage);

	FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
	HealthBarValue_TXT->SetText(FText::FromString(HealthText));
}
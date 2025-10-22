// Fill out your copyright notice in the Description page of Project Settings.

#include "UserInterface/WeaponInfoWidget.h"
#include "Components/TextBlock.h"


void UWeaponInfoWidget::UpdateMagazineAmmo(int32 MagazineAmmo)
{
	FString AmmoString = FString::Printf(TEXT("%d"), MagazineAmmo);
	MagazineAmmoValue_TXT->SetText(FText::FromString(AmmoString));
}


void UWeaponInfoWidget::UpdateReserveAmmo(int32 ReserveAmmo)
{
	FString AmmoString = FString::Printf(TEXT("%d"), ReserveAmmo);
	ReserveAmmoValue_TXT->SetText(FText::FromString(AmmoString));
}
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponInfoWidget.generated.h"

class UTextBlock;


UCLASS()
class MPARENASHOOTER_API UWeaponInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void UpdateMagazineAmmo(int32 MagazineAmmo);

	UFUNCTION(BlueprintCallable)
	void UpdateReserveAmmo(int32 ReserveAmmo);


private:

	UPROPERTY(meta = (BindWidget))
	UTextBlock *MagazineAmmoValue_TXT;

	UPROPERTY(meta = (BindWidget))
	UTextBlock *ReserveAmmoValue_TXT;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

class UTextBlock;


UCLASS()
class MPARENASHOOTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeDestruct() override;


	UFUNCTION(BlueprintCallable, Category = "OverheadWidget")
	void SetDisplayText(FString Text);

	UFUNCTION(BlueprintCallable, Category = "OverheadWidget")
	void DisplayPlayerNetRole(APawn *Pawn, bool bLocalRole = true);


private:

	UPROPERTY(meta = (BindWidget))
	UTextBlock *DisplayText_TXT;
};
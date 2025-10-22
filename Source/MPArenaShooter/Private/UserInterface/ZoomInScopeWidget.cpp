// Fill out your copyright notice in the Description page of Project Settings.

#include "UserInterface/ZoomInScopeWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"


void UZoomInScopeWidget::InitializeScope(bool bZoomIn, float AimValue)
{
	AddToViewport();

	if (AimValue > 0.f && AimValue < 1.f)
	{
		float AnimEndTime = Anim_ZoomIn->GetEndTime();
		float AnimStartTime = AimValue * AnimEndTime;
		int32 NumLoopsToPlay = 1;
		EUMGSequencePlayMode::Type PlayMode = bZoomIn ? EUMGSequencePlayMode::Forward : EUMGSequencePlayMode::Reverse;
		PlayAnimation(Anim_ZoomIn, AnimStartTime, NumLoopsToPlay, PlayMode);
	}
	else
	{
		ESlateVisibility WidgetVisibility = AimValue > 0.f ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
		SetVisibility(WidgetVisibility);
	}

	OnZoomOutCompleted.BindDynamic(this, &ThisClass::HideScope);
}


void UZoomInScopeWidget::ToggleZoom(bool bEnable, float ZoomSpeed)
{
	if (bEnable)
	{
		bZoomingOut = false;

		float CurrentTime = GetAnimationCurrentTime(Anim_ZoomIn);
		if (CurrentTime == 0.f)
		{
			SetVisibility(ESlateVisibility::Visible);
		}

		if (ZoomInSound)
		{
			UGameplayStatics::PlaySound2D(this, ZoomInSound);
		}

		UnbindFromAnimationFinished(Anim_ZoomIn, OnZoomOutCompleted);
		PlayAnimationForward(Anim_ZoomIn, ZoomSpeed);
	}
	else if (!bZoomingOut)
	{
		bZoomingOut = true;

		if (ZoomOutSound)
		{
			UGameplayStatics::PlaySound2D(this, ZoomOutSound);
		}

		BindToAnimationFinished(Anim_ZoomIn, OnZoomOutCompleted);
		PlayAnimationReverse(Anim_ZoomIn, ZoomSpeed);
	}
}


void UZoomInScopeWidget::HideScope()
{
	bZoomingOut = false;
	SetVisibility(ESlateVisibility::Hidden);
}
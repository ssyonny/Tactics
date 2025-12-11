// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerHUDWidget.h"
#include "TacticsCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Find health bar and HP text widgets
	HealthBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("HealthBar")));
	HPText = Cast<UTextBlock>(GetWidgetFromName(TEXT("HPDisplay")));
}

void UPlayerHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (TargetCharacter && !TargetCharacter->IsDead())
	{
		UpdateHealthDisplay();
	}
}

void UPlayerHUDWidget::SetCharacter(ATacticsCharacter* InCharacter)
{
	TargetCharacter = InCharacter;
	if (TargetCharacter)
	{
		UpdateHealthDisplay();
	}
}

void UPlayerHUDWidget::UpdateHealthDisplay()
{
	if (!TargetCharacter)
	{
		return;
	}

	float MaxHP = TargetCharacter->GetMaxHP();
	float CurrentHP = TargetCharacter->GetCurrentHP();

	if (MaxHP > 0.0f)
	{
		float HealthPercent = FMath::Clamp(CurrentHP / MaxHP, 0.0f, 1.0f);

		if (HealthBar)
		{
			HealthBar->SetPercent(HealthPercent);

			// Change color based on HP percentage
			if (HealthPercent > 0.5f)
			{
				HealthBar->SetFillColorAndOpacity(FLinearColor::Green);
			}
			else if (HealthPercent > 0.25f)
			{
				HealthBar->SetFillColorAndOpacity(FLinearColor::Yellow);
			}
			else
			{
				HealthBar->SetFillColorAndOpacity(FLinearColor::Red);
			}
		}

		if (HPText)
		{
			FString HPString = FString::Printf(TEXT("HP: %.0f / %.0f"), CurrentHP, MaxHP);
			HPText->SetText(FText::FromString(HPString));
		}
	}
}

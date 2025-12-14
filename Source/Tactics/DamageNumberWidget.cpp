// Copyright Epic Games, Inc. All Rights Reserved.

#include "DamageNumberWidget.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"

void UDamageNumberWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Add random horizontal offset for variety
	RandomOffset = FVector2D(FMath::RandRange(-30.0f, 30.0f), 0.0f);
}

void UDamageNumberWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update animation timer
	AnimationTimer += InDeltaTime;

	// Calculate animation progress (0 to 1)
	float Progress = FMath::Clamp(AnimationTimer / AnimationDuration, 0.0f, 1.0f);

	// Update position - float upward
	if (APlayerController* PC = GetOwningPlayer())
	{
		FVector2D ScreenPosition;
		if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, WorldLocation, ScreenPosition, false))
		{
			// Apply float up animation with ease out
			float EasedProgress = 1.0f - FMath::Pow(1.0f - Progress, 3.0f);
			FVector2D Offset = FVector2D(0.0f, -FloatHeight * EasedProgress) + RandomOffset;
			
			SetPositionInViewport(ScreenPosition + Offset);
		}
	}

	// Fade out in last 30% of animation
	if (Progress > 0.7f)
	{
		float FadeProgress = (Progress - 0.7f) / 0.3f;
		float Alpha = 1.0f - FadeProgress;
		SetRenderOpacity(Alpha);
	}

	// Remove widget when animation is complete
	if (Progress >= 1.0f)
	{
		RemoveFromParent();
	}
}

void UDamageNumberWidget::SetDamage(float Damage, bool bIsCritical)
{
	bCritical = bIsCritical;

	if (DamageText)
	{
		// Format damage as integer
		FString DamageString = FString::Printf(TEXT("%.0f"), Damage);
		
		// Add exclamation for critical hits
		if (bCritical)
		{
			DamageString += TEXT("!");
		}
		
		DamageText->SetText(FText::FromString(DamageString));
		
		// Set color based on critical hit
		FSlateColor Color = bCritical ? FSlateColor(CriticalColor) : FSlateColor(NormalColor);
		DamageText->SetColorAndOpacity(Color);
	}
}

void UDamageNumberWidget::SetWorldLocation(FVector Location)
{
	WorldLocation = Location;

	// Set initial screen position
	if (APlayerController* PC = GetOwningPlayer())
	{
		FVector2D ScreenPosition;
		if (UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, WorldLocation, ScreenPosition, false))
		{
			StartScreenPosition = ScreenPosition;
			SetPositionInViewport(ScreenPosition);
		}
	}
}

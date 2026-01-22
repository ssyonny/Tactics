// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "DamageNumberWidget.generated.h"

/**
 * Floating damage number widget - displays damage numbers above characters
 */
UCLASS()
class TACTICS_API UDamageNumberWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Initialize with damage amount and color */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void SetDamage(float Damage, bool bIsCritical = false);

	/** Set the world location to display at */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void SetWorldLocation(FVector Location);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** The text block to display damage */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DamageText;

	/** Animation duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float AnimationDuration = 1.0f;

	/** How high the number floats up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float FloatHeight = 100.0f;

	/** Normal damage color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor NormalColor = FLinearColor::White;

	/** Critical damage color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor CriticalColor = FLinearColor::Yellow;

private:
	/** World location to display at */
	FVector WorldLocation;

	/** Starting screen position */
	FVector2D StartScreenPosition;

	/** Timer for animation */
	float AnimationTimer = 0.0f;

	/** Is this a critical hit */
	bool bCritical = false;

	/** Initial random offset for variety */
	FVector2D RandomOffset;
};

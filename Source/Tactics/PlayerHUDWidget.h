// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "PlayerHUDWidget.generated.h"

class ATacticsCharacter;

/**
 * Player HUD Widget - Displays health bar and character info
 */
UCLASS()
class TACTICS_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Set the character to display */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetCharacter(ATacticsCharacter* InCharacter);

protected:
	/** Reference to the character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	ATacticsCharacter* TargetCharacter;

	/** Health bar widget */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UProgressBar* HealthBar;

	/** HP text widget */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UTextBlock* HPText;

	/** Update health display */
	void UpdateHealthDisplay();
};

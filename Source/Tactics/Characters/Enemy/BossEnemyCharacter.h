// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "BossEnemyCharacter.generated.h"

/**
 * Boss Enemy Character - Powerful enemy with special attacks and phases
 */
UCLASS(Blueprintable, BlueprintType)
class TACTICS_API ABossEnemyCharacter : public AEnemyCharacter
{
	GENERATED_BODY()

public:
	ABossEnemyCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Get current phase (1, 2, or 3) */
	UFUNCTION(BlueprintPure, Category = "Boss")
	int32 GetCurrentPhase() const { return CurrentPhase; }

	/** Check if boss is enraged */
	UFUNCTION(BlueprintPure, Category = "Boss")
	bool IsEnraged() const { return bIsEnraged; }

protected:
	/** Boss phases based on HP percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float Phase2HPThreshold = 0.6f; // 60% HP

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float Phase3HPThreshold = 0.3f; // 30% HP

	/** Enrage mode speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float EnrageSpeedMultiplier = 1.5f;

	/** Enrage mode damage multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float EnrageDamageMultiplier = 1.3f;

	/** Special attack cooldown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float SpecialAttackCooldown = 5.0f;

	/** Current phase */
	int32 CurrentPhase = 1;

	/** Is boss enraged */
	bool bIsEnraged = false;

	/** Special attack timer */
	float SpecialAttackTimer = 0.0f;

	/** Override attack with boss attacks */
	virtual void AttackPlayer() override;

	/** Perform special attack based on phase */
	UFUNCTION(BlueprintNativeEvent, Category = "Boss")
	void PerformSpecialAttack();
	virtual void PerformSpecialAttack_Implementation();

	/** Called when entering a new phase */
	UFUNCTION(BlueprintNativeEvent, Category = "Boss")
	void OnPhaseChange(int32 NewPhase);
	virtual void OnPhaseChange_Implementation(int32 NewPhase);

	/** Check and update phase based on HP */
	void UpdatePhase();

	/** Override death for boss-specific effects */
	virtual void OnDeath_Implementation() override;
};

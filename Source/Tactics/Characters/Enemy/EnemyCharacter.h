// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TacticsCharacter.h"
#include "EnemyCharacter.generated.h"

/**
 * Enemy Character - AI controlled enemy that can patrol, chase, and attack
 */
UCLASS(Blueprintable, BlueprintType)
class TACTICS_API AEnemyCharacter : public ATacticsCharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Check if player is in detection range */
	UFUNCTION(BlueprintPure, Category = "AI")
	bool IsPlayerInRange() const;

	/** Check if player is in attack range */
	UFUNCTION(BlueprintPure, Category = "AI")
	bool IsPlayerInAttackRange() const;

	/** Get direction to player */
	UFUNCTION(BlueprintPure, Category = "AI")
	FVector GetDirectionToPlayer() const;

	/** Get distance to player */
	UFUNCTION(BlueprintPure, Category = "AI")
	float GetDistanceToPlayer() const;

	/** Attack the player */
	UFUNCTION(BlueprintCallable, Category = "AI")
	virtual void AttackPlayer();

protected:
	/** Detection range - how far the enemy can see the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float DetectionRange = 1000.0f;

	/** Attack range - how close the enemy needs to be to attack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float EnemyAttackRange = 150.0f;

	/** Movement speed when chasing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float ChaseSpeed = 400.0f;

	/** Movement speed when patrolling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float PatrolSpeed = 200.0f;

	/** Time between attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AttackInterval = 1.5f;

	/** Points awarded when killed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 ScoreValue = 100;

	/** Experience points awarded when killed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 ExpValue = 25;

	/** Cached reference to player character */
	UPROPERTY()
	class ATacticsCharacter* PlayerCharacter;

	/** Timer for attack cooldown */
	float AttackCooldownTimer = 0.0f;

	/** Called when this enemy dies */
	virtual void OnDeath_Implementation() override;

private:
	/** Find and cache player reference */
	void FindPlayer();

	/** Simple AI behavior - chase and attack */
	void UpdateAI(float DeltaSeconds);
};

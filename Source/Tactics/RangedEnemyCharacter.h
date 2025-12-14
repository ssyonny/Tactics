// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "RangedEnemyCharacter.generated.h"

/**
 * Ranged Enemy Character - Attacks from a distance with projectiles
 */
UCLASS(Blueprintable, BlueprintType)
class TACTICS_API ARangedEnemyCharacter : public AEnemyCharacter
{
	GENERATED_BODY()

public:
	ARangedEnemyCharacter();

protected:
	/** Projectile class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<class AActor> ProjectileClass;

	/** Projectile spawn offset from character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FVector ProjectileSpawnOffset = FVector(50.0f, 0.0f, 50.0f);

	/** Projectile speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float ProjectileSpeed = 1000.0f;

	/** Override attack to fire projectile instead */
	virtual void AttackPlayer() override;
};

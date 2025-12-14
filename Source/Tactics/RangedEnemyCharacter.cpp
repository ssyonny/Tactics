// Copyright Epic Games, Inc. All Rights Reserved.

#include "RangedEnemyCharacter.h"
#include "Tactics.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"

ARangedEnemyCharacter::ARangedEnemyCharacter()
{
	// Ranged enemies have longer attack range but same detection
	EnemyAttackRange = 600.0f;
	DetectionRange = 1200.0f;
	
	// Longer attack interval for balance
	AttackInterval = 2.0f;
	
	// Lower HP than melee
	MaxHP = 60.0f;
	BaseDamage = 15.0f;
	
	// More points for ranged enemies
	ScoreValue = 150;
	ExpValue = 40;
}

void ARangedEnemyCharacter::AttackPlayer()
{
	if (!PlayerCharacter || PlayerCharacter->IsDead())
	{
		return;
	}

	// Check cooldown
	if (AttackCooldownTimer > 0.0f)
	{
		return;
	}

	// Face the player
	FVector DirectionToPlayer = GetDirectionToPlayer();
	ForceRotateToDirection(DirectionToPlayer);

	// Play attack animation
	PlayAttackAnimation();

	// Spawn projectile if class is set
	if (ProjectileClass && GetWorld())
	{
		// Calculate spawn location
		FVector SpawnLocation = GetActorLocation() + GetActorRotation().RotateVector(ProjectileSpawnOffset);
		FRotator SpawnRotation = DirectionToPlayer.Rotation();
		
		// Spawn projectile
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		
		AActor* Projectile = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
		
		if (Projectile)
		{
			// If projectile has ProjectileMovementComponent, set velocity
			if (UProjectileMovementComponent* ProjMovement = Projectile->FindComponentByClass<UProjectileMovementComponent>())
			{
				ProjMovement->Velocity = DirectionToPlayer * ProjectileSpeed;
			}
			
			UE_LOG(LogTactics, Log, TEXT("%s fired projectile at player"), *GetName());
		}
	}
	else
	{
		// Fallback: Apply damage directly if no projectile class (like hitscan)
		if (IsPlayerInAttackRange())
		{
			float Damage = CalculateDamage(0.0f);
			UGameplayStatics::ApplyDamage(PlayerCharacter, Damage, GetController(), this, nullptr);
			
			UE_LOG(LogTactics, Log, TEXT("%s hit player with ranged attack for %f damage"), *GetName(), Damage);
		}
	}

	// Set cooldown
	AttackCooldownTimer = AttackInterval;
}

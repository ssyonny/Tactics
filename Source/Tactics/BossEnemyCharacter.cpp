// Copyright Epic Games, Inc. All Rights Reserved.

#include "BossEnemyCharacter.h"
#include "Tactics.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

ABossEnemyCharacter::ABossEnemyCharacter()
{
	// Boss has much higher HP
	MaxHP = 500.0f;
	BaseDamage = 25.0f;
	
	// Boss is slower but has larger range
	ChaseSpeed = 300.0f;
	PatrolSpeed = 150.0f;
	EnemyAttackRange = 200.0f;
	DetectionRange = 1500.0f;
	
	// Faster attacks
	AttackInterval = 1.0f;
	
	// Much more rewards
	ScoreValue = 1000;
	ExpValue = 250;
}

void ABossEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Start at phase 1
	CurrentPhase = 1;
	bIsEnraged = false;
}

void ABossEnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Update special attack timer
	if (SpecialAttackTimer > 0.0f)
	{
		SpecialAttackTimer -= DeltaSeconds;
	}

	// Check for phase changes
	UpdatePhase();
}

void ABossEnemyCharacter::UpdatePhase()
{
	if (IsDead())
	{
		return;
	}

	float HPPercent = CurrentHP / MaxHP;
	int32 NewPhase = CurrentPhase;

	if (HPPercent <= Phase3HPThreshold)
	{
		NewPhase = 3;
	}
	else if (HPPercent <= Phase2HPThreshold)
	{
		NewPhase = 2;
	}
	else
	{
		NewPhase = 1;
	}

	if (NewPhase != CurrentPhase)
	{
		CurrentPhase = NewPhase;
		OnPhaseChange(CurrentPhase);
	}
}

void ABossEnemyCharacter::OnPhaseChange_Implementation(int32 NewPhase)
{
	UE_LOG(LogTactics, Warning, TEXT("BOSS %s entered Phase %d!"), *GetName(), NewPhase);

	// Enrage in phase 3
	if (NewPhase == 3)
	{
		bIsEnraged = true;
		
		// Increase speed
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed * EnrageSpeedMultiplier;
		}
		
		// Increase damage
		BaseDamage *= EnrageDamageMultiplier;
		
		UE_LOG(LogTactics, Warning, TEXT("BOSS %s is now ENRAGED!"), *GetName());
	}
}

void ABossEnemyCharacter::AttackPlayer()
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

	// Try special attack if available
	if (SpecialAttackTimer <= 0.0f && CurrentPhase >= 2)
	{
		PerformSpecialAttack();
		SpecialAttackTimer = SpecialAttackCooldown;
		AttackCooldownTimer = AttackInterval;
		return;
	}

	// Normal attack
	PerformAttack();

	// Apply damage
	if (IsPlayerInAttackRange())
	{
		float Damage = CalculateDamage(0.0f);
		UGameplayStatics::ApplyDamage(PlayerCharacter, Damage, GetController(), this, nullptr);
		
		UE_LOG(LogTactics, Log, TEXT("BOSS %s attacked player for %f damage"), *GetName(), Damage);
	}

	// Set cooldown
	AttackCooldownTimer = AttackInterval;
}

void ABossEnemyCharacter::PerformSpecialAttack_Implementation()
{
	UE_LOG(LogTactics, Warning, TEXT("BOSS %s performs SPECIAL ATTACK (Phase %d)!"), *GetName(), CurrentPhase);

	// Play attack animation
	PlayAttackAnimation();

	// Deal extra damage in a wider area
	float SpecialDamage = BaseDamage * 2.0f;
	float SpecialRange = EnemyAttackRange * 1.5f;

	if (GetDistanceToPlayer() <= SpecialRange)
	{
		UGameplayStatics::ApplyDamage(PlayerCharacter, SpecialDamage, GetController(), this, nullptr);
		
		UE_LOG(LogTactics, Log, TEXT("BOSS special attack hit player for %f damage!"), SpecialDamage);
	}

	// TODO: Add visual effects for special attack
	// TODO: Add screen shake
}

void ABossEnemyCharacter::OnDeath_Implementation()
{
	UE_LOG(LogTactics, Warning, TEXT("=== BOSS %s DEFEATED! ==="), *GetName());
	UE_LOG(LogTactics, Warning, TEXT("Score: %d, Exp: %d"), ScoreValue, ExpValue);

	// Call parent implementation
	Super::OnDeath_Implementation();

	// TODO: Spawn epic loot
	// TODO: Play victory fanfare
	// TODO: Trigger cutscene or level transition
}

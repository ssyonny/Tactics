// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyCharacter.h"
#include "Tactics.h"
#include "TacticsCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

AEnemyCharacter::AEnemyCharacter()
{
	// Enable tick for AI updates
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Set default enemy stats (higher HP for testing)
	MaxHP = 100.0f;
	BaseDamage = 10.0f;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize HP
	CurrentHP = MaxHP;

	// Configure movement for AI control
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
		// Disable auto-rotation so we can control it smoothly in UpdateAI
		GetCharacterMovement()->bOrientRotationToMovement = false;
		GetCharacterMovement()->bUseControllerDesiredRotation = false;
	}

	// Find player with a small delay to ensure player is spawned
	FTimerHandle FindPlayerTimer;
	GetWorld()->GetTimerManager().SetTimer(FindPlayerTimer, this, &AEnemyCharacter::FindPlayer, 0.5f, false);
}

void AEnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Update attack cooldown
	if (AttackCooldownTimer > 0.0f)
	{
		AttackCooldownTimer -= DeltaSeconds;
	}

	// Run simple AI
	UpdateAI(DeltaSeconds);
}

void AEnemyCharacter::FindPlayer()
{
	// Find player character
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (PlayerPawn)
	{
		PlayerCharacter = Cast<ATacticsCharacter>(PlayerPawn);
		if (PlayerCharacter)
		{
			UE_LOG(LogTactics, Log, TEXT("%s found player: %s"), *GetName(), *PlayerCharacter->GetName());
		}
	}
}

bool AEnemyCharacter::IsPlayerInRange() const
{
	if (!PlayerCharacter || PlayerCharacter->IsDead())
	{
		return false;
	}

	return GetDistanceToPlayer() <= DetectionRange;
}

bool AEnemyCharacter::IsPlayerInAttackRange() const
{
	if (!PlayerCharacter || PlayerCharacter->IsDead())
	{
		return false;
	}

	return GetDistanceToPlayer() <= EnemyAttackRange;
}

FVector AEnemyCharacter::GetDirectionToPlayer() const
{
	if (!PlayerCharacter)
	{
		return FVector::ForwardVector;
	}

	FVector Direction = PlayerCharacter->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.0f; // Keep horizontal
	return Direction.GetSafeNormal();
}

float AEnemyCharacter::GetDistanceToPlayer() const
{
	if (!PlayerCharacter)
	{
		return FLT_MAX;
	}

	return FVector::Distance(GetActorLocation(), PlayerCharacter->GetActorLocation());
}

void AEnemyCharacter::AttackPlayer()
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

	// Perform attack (uses parent class attack logic)
	PerformAttack();

	// Apply damage to player if in range using UGameplayStatics
	if (IsPlayerInAttackRange())
	{
		float Damage = CalculateDamage(0.0f); // TODO: Get player armor
		UGameplayStatics::ApplyDamage(PlayerCharacter, Damage, GetController(), this, nullptr);
		
		UE_LOG(LogTactics, Log, TEXT("%s attacked player for %f damage"), *GetName(), Damage);
	}

	// Set cooldown
	AttackCooldownTimer = AttackInterval;
}

void AEnemyCharacter::UpdateAI(float DeltaSeconds)
{
	// Skip if dead or no player
	if (IsDead() || !PlayerCharacter || PlayerCharacter->IsDead())
	{
		return;
	}

	// Check if player is in detection range
	if (IsPlayerInRange())
	{
		// Set chase speed
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
		}

		// Always face the player while chasing
		FVector DirectionToPlayer = GetDirectionToPlayer();
		
		// Smooth rotation towards player
		FRotator CurrentRotation = GetActorRotation();
		FRotator TargetRotation = DirectionToPlayer.Rotation();
		FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaSeconds, 10.0f);
		SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));

		// Check if in attack range
		if (IsPlayerInAttackRange())
		{
			// Stop moving and attack
			AttackPlayer();
		}
		else
		{
			// Move towards player
			AddMovementInput(DirectionToPlayer, 1.0f);
		}
	}
	else
	{
		// Set patrol speed when not chasing
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
		}

		// TODO: Implement patrol behavior
	}
}

void AEnemyCharacter::OnDeath_Implementation()
{
	// Call parent implementation first (disables movement and collision)
	Super::OnDeath_Implementation();

	UE_LOG(LogTactics, Log, TEXT("%s killed! Score: %d, Exp: %d"), *GetName(), ScoreValue, ExpValue);

	// TODO: Award score and experience to player
	// TODO: Spawn death effects or loot

	// Fade out the mesh instead of instant disappear
	if (GetMesh())
	{
		// Set mesh to be rendered in translucent mode for fade
		GetMesh()->SetScalarParameterValueOnMaterials(TEXT("Opacity"), 0.5f);
	}

	// Destroy after longer delay (5 seconds instead of 3)
	SetLifeSpan(5.0f);
}

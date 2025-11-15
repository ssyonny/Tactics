// Copyright Epic Games, Inc. All Rights Reserved.

#include "TacticsCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"

ATacticsCharacter::ATacticsCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create the camera boom component
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;

	// Create the camera component
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));

	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ATacticsCharacter::BeginPlay()
{
	Super::BeginPlay();

	// stub
}

void ATacticsCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	// Update attack cooldown
	if (CurrentAttackCooldown > 0.0f)
	{
		CurrentAttackCooldown -= DeltaSeconds;
	}
}

void ATacticsCharacter::PerformAttack()
{
	if (!CanAttack())
	{
		return;
	}

	// Set cooldown
	CurrentAttackCooldown = AttackCooldown;

	// Perform sphere trace to detect enemies in range
	FVector StartLocation = GetActorLocation();
	FVector EndLocation = StartLocation + (GetActorForwardVector() * AttackRange);

	// Setup collision query params
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	// Perform sphere trace
	TArray<FHitResult> HitResults;
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECollisionChannel::ECC_Pawn,
		FCollisionShape::MakeSphere(50.0f),
		QueryParams
	);

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			if (AActor* HitActor = Hit.GetActor())
			{
				// Calculate damage (assume 0 armor for now, will be updated later)
				float Damage = CalculateDamage(0.0f);
				
				// Apply damage
				// TODO: Implement damage system
				UE_LOG(LogTactics, Log, TEXT("Hit %s with %f damage"), *HitActor->GetName(), Damage);
			}
		}
	}

	// Play attack animation
	// TODO: Add animation montage
	UE_LOG(LogTactics, Log, TEXT("Attack performed! Cooldown: %f"), AttackCooldown);
}

bool ATacticsCharacter::CanAttack() const
{
	return CurrentAttackCooldown <= 0.0f;
}

float ATacticsCharacter::CalculateDamage(float TargetArmor) const
{
	// Damage formula: DamageTaken = round(BaseDamage * 100/(100+Armor))
	float DamageMultiplier = 100.0f / (100.0f + TargetArmor);
	return FMath::RoundToFloat(BaseDamage * DamageMultiplier);
}

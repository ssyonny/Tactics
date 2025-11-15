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
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "UObject/ConstructorHelpers.h"

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

	// Play attack animation
	PlayAttackAnimation();

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
	PlayAttackAnimation();
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

void ATacticsCharacter::PlayAttackAnimation()
{
	// Select random attack animation
	UAnimMontage* SelectedMontage = nullptr;
	int32 RandomAttack = FMath::RandRange(1, 3);
	
	switch (RandomAttack)
	{
		case 1:
			SelectedMontage = AttackMontage1;
			break;
		case 2:
			SelectedMontage = AttackMontage2;
			break;
		case 3:
			SelectedMontage = AttackMontage3;
			break;
	}

	if (SelectedMontage && GetMesh() && GetMesh()->GetAnimInstance())
	{
		UE_LOG(LogTactics, Warning, TEXT("Playing attack animation: %s"), *SelectedMontage->GetName());
		GetMesh()->GetAnimInstance()->Montage_Play(SelectedMontage);
	}
	else
	{
		UE_LOG(LogTactics, Warning, TEXT("No attack animation available, using fallback"));
		// If no animation, the attack logic already executed above
	}
}

void ATacticsCharacter::OnAttackAnimationFinished()
{
	UE_LOG(LogTactics, Warning, TEXT("Attack animation finished"));
	// This function can be called from animation blueprint when montage finishes
	// Additional logic can be added here if needed
}

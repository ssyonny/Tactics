// Copyright Epic Games, Inc. All Rights Reserved.

#include "TacticsCharacter.h"
#include "Tactics.h"

// Core
#include "Engine/World.h"
#include "Engine/DamageEvents.h"

// Gameplay
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Components
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Components/CapsuleComponent.h"

// Animation
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

// Niagara VFX
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

ATacticsCharacter::ATacticsCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // 이동 시 자연스러운 회전 허용
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f); // 적당한 회전 속도 복원
	GetCharacterMovement()->bUseControllerDesiredRotation = false; // Controller 회전은 비활성화 유지
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

	// Create Niagara trail component
	TrailComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AttackTrail"));
	TrailComponent->SetupAttachment(GetMesh()); // 메시 중심에 부착
	TrailComponent->SetAutoActivate(false); // 기본적으로 비활성화
}

void ATacticsCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize HP
	CurrentHP = MaxHP;

	// TODO: Register with HUD when HUD system is implemented
}

void ATacticsCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	// Update attack cooldown
	if (CurrentAttackCooldown > 0.0f)
	{
		CurrentAttackCooldown -= DeltaSeconds;
		if (CurrentAttackCooldown < 0.f)
			CurrentAttackCooldown = 0.f;
	}
}

void ATacticsCharacter::TestPerformAttack()
{
	// Debug function - kept for future testing if needed
	UE_LOG(LogTactics, Verbose, TEXT("TestPerformAttack called"));
}

void ATacticsCharacter::EmergencyAttackTest()
{
	// Debug function - kept for future testing if needed
	UE_LOG(LogTactics, Verbose, TEXT("EmergencyAttackTest called"));
}

void ATacticsCharacter::PerformAttack()
{
	UE_LOG(LogTactics, Verbose, TEXT("PerformAttack called"));

	// Safety check for CDO and invalid state
	if (!GetWorld())
	{
		return;
	}

	// Play attack sound
	if (AttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
	}

	if (!CanAttack())
	{
		UE_LOG(LogTactics, Verbose, TEXT("Attack blocked by cooldown. Remaining: %f"), CurrentAttackCooldown);
		return;
	}

	// Set cooldown
	CurrentAttackCooldown = AttackCooldown;

	// Get attack direction based on mouse position and store it
	LastAttackDirection = GetAttackDirection();

	// Force immediate rotation to attack direction
	ForceRotateToDirection(LastAttackDirection);

	// Play attack animation and effects AFTER rotation
	PlayAttackAnimation();
	SpawnAttackEffects();

	// Perform sphere trace to detect enemies in range using stored attack direction
	FVector StartLocation = GetActorLocation();
	FVector EndLocation = StartLocation + (LastAttackDirection * AttackRange);

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
				
			
			// Apply damage to target if it's a TacticsCharacter
			if (ATacticsCharacter* TargetCharacter = Cast<ATacticsCharacter>(HitActor))
			{
				FDamageEvent DamageEvent;
				TargetCharacter->TakeDamage(Damage, DamageEvent, GetController(), this);
			}				UE_LOG(LogTactics, Log, TEXT("Hit %s with %f damage"), *HitActor->GetName(), Damage);

				// 피격 사운드 재생
				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, HitSound, HitActor->GetActorLocation());
				}

				// Spawn impact effect at hit location
				SpawnImpactEffect(Hit.Location);
			}
		}
	}

	// Add camera shake for impact feedback
	if (AttackCameraShake)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->ClientStartCameraShake(AttackCameraShake);
		}
	}
	
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

float ATacticsCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (IsDead())
	{
		return 0.0f;
	}

	float ActualDamage = FMath::Min(DamageAmount, CurrentHP);
	CurrentHP -= ActualDamage;

	UE_LOG(LogTactics, Log, TEXT("%s took %f damage. HP: %f/%f"), *GetName(), ActualDamage, CurrentHP, MaxHP);

	// Show damage number on HUD
	// TODO: Implement HUD damage numbers later
	/*
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (ATacticsHUD* TacticsHUD = Cast<ATacticsHUD>(PC->GetHUD()))
		{
			TacticsHUD->ShowDamageNumber(ActualDamage, GetActorLocation());
		}
	}
	*/

	if (IsDead())
	{
		OnDeath();
	}

	return ActualDamage;
}

void ATacticsCharacter::Heal(float HealAmount)
{
	if (IsDead())
	{
		return;
	}

	float OldHP = CurrentHP;
	CurrentHP = FMath::Min(CurrentHP + HealAmount, MaxHP);
	
	UE_LOG(LogTactics, Log, TEXT("%s healed %f. HP: %f/%f"), *GetName(), CurrentHP - OldHP, CurrentHP, MaxHP);
}

bool ATacticsCharacter::IsDead() const
{
	return CurrentHP <= 0.0f;
}

void ATacticsCharacter::OnDeath_Implementation()
{
	UE_LOG(LogTactics, Log, TEXT("%s has died!"), *GetName());
	
	// Disable movement
	GetCharacterMovement()->DisableMovement();
	
	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Play death animation
	if (DeathMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(DeathMontage);
			UE_LOG(LogTactics, Verbose, TEXT("Playing death animation: %s"), *DeathMontage->GetName());
		}
	}
	else
	{
		UE_LOG(LogTactics, Verbose, TEXT("No death animation set"));
	}
	
	// TODO: Spawn death effects, handle respawn, etc.
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
		UE_LOG(LogTactics, Verbose, TEXT("Playing attack animation: %s"), *SelectedMontage->GetName());
		GetMesh()->GetAnimInstance()->Montage_Play(SelectedMontage);
	}
	else
	{
		UE_LOG(LogTactics, Verbose, TEXT("No attack animation available"));
	}
}

void ATacticsCharacter::OnAttackAnimationFinished()
{
	// This function can be called from animation blueprint when montage finishes
}

void ATacticsCharacter::SpawnAttackEffects()
{
	// Safety check
	if (!GetWorld())
	{
		return;
	}
	
	// Activate trail effect during attack
	if (TrailComponent && AttackTrailEffect)
	{
		TrailComponent->SetAsset(AttackTrailEffect);
		TrailComponent->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.0f));
		
		// Align trail with attack direction (world space - no mesh offset needed)
		FRotator TrailRotation = LastAttackDirection.Rotation();
		TrailComponent->SetWorldRotation(TrailRotation);
		
		TrailComponent->Activate(true);
		
		// Deactivate trail after duration
		FTimerHandle TrailTimer;
		GetWorld()->GetTimerManager().SetTimer(TrailTimer, [this]()
		{
			if (IsValid(this) && TrailComponent)
			{
				TrailComponent->Deactivate();
			}
		}, 2.0f, false);
	}
}

void ATacticsCharacter::SpawnImpactEffect(FVector Location)
{
	// Spawn impact effect at hit location
	if (AttackImpactEffect && GetWorld())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			AttackImpactEffect,
			Location,
			FRotator::ZeroRotator,
			FVector(3.0f, 3.0f, 3.0f)
		);
	}
}

FVector ATacticsCharacter::GetAttackDirection() const
{
	// Get player controller
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// Get mouse cursor position in world space
		FVector MouseWorldLocation, MouseWorldDirection;
		if (PC->DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection))
		{
			// Create a plane at character's Z level
			FPlane GroundPlane(GetActorLocation(), FVector::UpVector);
			
			// Find intersection with ground plane
			FVector IntersectionPoint = FMath::LinePlaneIntersection(
				MouseWorldLocation,
				MouseWorldLocation + MouseWorldDirection * 10000.0f,
				GroundPlane
			);
			
			// Calculate direction from character to mouse position
			FVector AttackDirection = (IntersectionPoint - GetActorLocation()).GetSafeNormal();
			AttackDirection.Z = 0.0f; // Keep it horizontal
			
			// Ensure we have a valid direction
			if (AttackDirection.SizeSquared() < 0.1f)
			{
				AttackDirection = GetActorForwardVector();
			}
			
			return AttackDirection;
		}
	}
	
	// Fallback to forward vector
	return GetActorForwardVector();
}

void ATacticsCharacter::ForceRotateToDirection(FVector Direction)
{
	if (Direction.IsZero())
	{
		return;
	}
	
	// Safety check for CDO and invalid state
	if (!GetWorld() || !GetCharacterMovement())
	{
		return;
	}
	
	// Calculate target rotation from direction
	// Note: MeshYawOffset compensates for mesh forward direction mismatch
	// Most character meshes face +Y (right) instead of +X (forward) in their default pose
	FRotator TargetRotation = Direction.Rotation();
	TargetRotation.Yaw += MeshYawOffset; // Compensate for mesh orientation
	
	// Temporarily disable movement-based rotation for precise control
	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	bool bOriginalOrientRotation = MovementComp->bOrientRotationToMovement;
	MovementComp->bOrientRotationToMovement = false;
	
	// Apply immediate rotation
	SetActorRotation(TargetRotation);
	
	// Restore movement rotation after a short delay
	FTimerHandle RestoreMovementTimer;
	GetWorld()->GetTimerManager().SetTimer(RestoreMovementTimer, [this, bOriginalOrientRotation]()
	{
		if (IsValid(this) && GetCharacterMovement())
		{
			GetCharacterMovement()->bOrientRotationToMovement = bOriginalOrientRotation;
		}
	}, 0.2f, false);
}


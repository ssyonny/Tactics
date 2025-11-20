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
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogTactics, Log, All);

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

	// stub
}

void ATacticsCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	// 쿨타임 상태 실시간 로그
	UE_LOG(LogTactics, Warning, TEXT("[TICK] CurrentAttackCooldown: %f"), CurrentAttackCooldown);
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
	UE_LOG(LogTactics, Warning, TEXT("=== TEST PERFORMATTACK FUNCTION ENTERED ==="));
	UE_LOG(LogTactics, Warning, TEXT("TestPerformAttack called! This proves C++ code is working!"));
}

void ATacticsCharacter::EmergencyAttackTest()
{
	UE_LOG(LogTactics, Warning, TEXT("=== EMERGENCY ATTACK TEST WORKING! ==="));
	UE_LOG(LogTactics, Warning, TEXT("C++ CODE IS DEFINITELY WORKING!"));
}

void ATacticsCharacter::PerformAttack()
{
	UE_LOG(LogTactics, Warning, TEXT("=== PERFORMATTACK FUNCTION ENTERED ==="));
	UE_LOG(LogTactics, Warning, TEXT("PerformAttack called! CurrentCooldown: %f"), CurrentAttackCooldown);

	// 쿨타임과 상관없이 항상 마우스/캐릭터 방향 로그 출력
	FVector MouseWorldLocation, MouseWorldDirection, MouseIntersection(0,0,0);
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection))
		{
			FPlane GroundPlane(GetActorLocation(), FVector::UpVector);
			MouseIntersection = FMath::LinePlaneIntersection(
				MouseWorldLocation,
				MouseWorldLocation + MouseWorldDirection * 10000.0f,
				GroundPlane
			);
		}
	}
	FVector TempAttackDirection = (MouseIntersection - GetActorLocation()).GetSafeNormal();
	TempAttackDirection.Z = 0.0f;
	UE_LOG(LogTactics, Warning, TEXT("[DEBUG] MouseWorldLocation: %s"), *MouseWorldLocation.ToString());
	UE_LOG(LogTactics, Warning, TEXT("[DEBUG] MouseIntersection(Ground): %s"), *MouseIntersection.ToString());
	UE_LOG(LogTactics, Warning, TEXT("[DEBUG] TempAttackDirection: %s"), *TempAttackDirection.ToString());
	UE_LOG(LogTactics, Warning, TEXT("[DEBUG] Actor Current Forward: %s"), *GetActorForwardVector().ToString());
	UE_LOG(LogTactics, Warning, TEXT("[DEBUG] Actor Current Rotation: %s"), *GetActorRotation().ToString());

	if (!CanAttack())
	{
		UE_LOG(LogTactics, Warning, TEXT("Attack blocked by cooldown! Remaining: %f"), CurrentAttackCooldown);
		return;
	}

	UE_LOG(LogTactics, Warning, TEXT("Attack allowed! Proceeding..."));

	// Set cooldown
	CurrentAttackCooldown = AttackCooldown;

	// Get attack direction based on mouse position and store it
	LastAttackDirection = GetAttackDirection();

	// Debug the directions and positions (공격 허용 시)
	UE_LOG(LogTactics, Warning, TEXT("=== ATTACK DIRECTION DEBUG ==="));
	UE_LOG(LogTactics, Warning, TEXT("Raw Attack Direction: %s"), *LastAttackDirection.ToString());
	UE_LOG(LogTactics, Warning, TEXT("Actor Current Forward: %s"), *GetActorForwardVector().ToString());
	UE_LOG(LogTactics, Warning, TEXT("Actor Current Rotation: %s"), *GetActorRotation().ToString());

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
				
				// Apply damage
				// TODO: Implement damage system
				UE_LOG(LogTactics, Log, TEXT("Hit %s with %f damage"), *HitActor->GetName(), Damage);
				
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
	bool bCanAttack = CurrentAttackCooldown <= 0.0f;
	UE_LOG(LogTactics, Warning, TEXT("CanAttack: %s (Cooldown: %f)"), bCanAttack ? TEXT("TRUE") : TEXT("FALSE"), CurrentAttackCooldown);
	return bCanAttack;
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

void ATacticsCharacter::SpawnAttackEffects()
{
	// Activate trail effect during attack
	if (TrailComponent && AttackTrailEffect)
	{
		TrailComponent->SetAsset(AttackTrailEffect);
		TrailComponent->SetRelativeScale3D(FVector(2.0f, 2.0f, 2.0f)); // 2배 크기
		
		// Use stored last attack direction for perfect consistency
		FRotator TrailRotation = LastAttackDirection.Rotation();
		
		// Set world rotation instead of relative rotation for accuracy
		TrailComponent->SetWorldRotation(TrailRotation);
		
		TrailComponent->Activate(true);
		
		// Debug logs
		UE_LOG(LogTactics, Warning, TEXT("=== TRAIL EFFECTS DEBUG ==="));
		UE_LOG(LogTactics, Warning, TEXT("Stored Attack Direction: %s"), *LastAttackDirection.ToString());
		UE_LOG(LogTactics, Warning, TEXT("Trail Rotation (from direction): %s"), *TrailRotation.ToString());
		UE_LOG(LogTactics, Warning, TEXT("Actor Rotation: %s"), *GetActorRotation().ToString());
		UE_LOG(LogTactics, Warning, TEXT("Actor Forward After Rotation: %s"), *GetActorForwardVector().ToString());
		
		// Compare if they match
		FVector ActorForward = GetActorForwardVector();
		float DotProduct = FVector::DotProduct(LastAttackDirection, ActorForward);
		UE_LOG(LogTactics, Warning, TEXT("Direction Match (1.0 = perfect): %f"), DotProduct);
		
		// Deactivate trail after longer duration for better visibility
		FTimerHandle TrailTimer;
		GetWorld()->GetTimerManager().SetTimer(TrailTimer, [this]()
		{
			if (TrailComponent)
			{
				TrailComponent->Deactivate();
			}
		}, 2.0f, false); // 2초로 늘림
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
			FVector(3.0f, 3.0f, 3.0f) // 3배 크기로 더 잘 보이게
		);
		UE_LOG(LogTactics, Warning, TEXT("Impact effect spawned at location: %s"), *Location.ToString());
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
			
			// Debug log
			UE_LOG(LogTactics, Warning, TEXT("Mouse Intersection: %s, Attack Direction: %s"), 
				*IntersectionPoint.ToString(), *AttackDirection.ToString());
			
			return AttackDirection;
		}
	}
	
	// Fallback to forward vector
	UE_LOG(LogTactics, Warning, TEXT("Using fallback forward vector"));
	return GetActorForwardVector();
}

void ATacticsCharacter::ForceRotateToDirection(FVector Direction)
{
	if (Direction.IsZero())
	{
		return;
	}
	
	// Test different rotation corrections to find the right one
	FRotator BaseRotation = Direction.Rotation();
	FRotator TestRotations[4] = {
		BaseRotation,                    // 0도 (원본)
		BaseRotation + FRotator(0, 90, 0),   // +90도
		BaseRotation + FRotator(0, 180, 0),  // +180도
		BaseRotation + FRotator(0, -90, 0)   // -90도
	};
	
	UE_LOG(LogTactics, Warning, TEXT("=== ROTATION TESTING ==="));
	UE_LOG(LogTactics, Warning, TEXT("Direction Vector: %s"), *Direction.ToString());
	UE_LOG(LogTactics, Warning, TEXT("Base Rotation (0°): %f"), BaseRotation.Yaw);
	UE_LOG(LogTactics, Warning, TEXT("Test +90°: %f"), TestRotations[1].Yaw);
	UE_LOG(LogTactics, Warning, TEXT("Test +180°: %f"), TestRotations[2].Yaw);
	UE_LOG(LogTactics, Warning, TEXT("Test -90°: %f"), TestRotations[3].Yaw);
	
	// Use the original rotation for now to see which one is correct
	FRotator TargetRotation = BaseRotation;
	
	// Temporarily disable movement-based rotation for precise control
	bool bOriginalOrientRotation = GetCharacterMovement()->bOrientRotationToMovement;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	
	// Apply immediate rotation
	SetActorRotation(TargetRotation);
	
	// Verify and log the rotation
	FRotator CurrentRotation = GetActorRotation();
	
	UE_LOG(LogTactics, Warning, TEXT("APPLIED - Target Yaw: %f, Current Yaw: %f"), 
		TargetRotation.Yaw, CurrentRotation.Yaw);
	UE_LOG(LogTactics, Warning, TEXT("New Forward Vector: %s"), *GetActorForwardVector().ToString());
	
	// Restore movement rotation after a short delay
	FTimerHandle RestoreMovementTimer;
	GetWorld()->GetTimerManager().SetTimer(RestoreMovementTimer, [this, bOriginalOrientRotation]()
	{
		GetCharacterMovement()->bOrientRotationToMovement = bOriginalOrientRotation;
	}, 0.2f, false);
}

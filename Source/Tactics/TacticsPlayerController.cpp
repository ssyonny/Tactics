// Copyright Epic Games, Inc. All Rights Reserved.

#include "TacticsPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "TacticsCharacter.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/LocalPlayer.h"
#include "Tactics.h"

ATacticsPlayerController::ATacticsPlayerController()
{
	bIsTouch = false;
	bMoveToMouseCursor = false;

	// configure the controller
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;

	// Fallback: Auto-load input assets if not set via Blueprint
	// Mapping Context
	if (!DefaultMappingContext)
	{
		static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMC(TEXT("/Game/TopDown/Input/IMC_Default.IMC_Default"));
		if (IMC.Succeeded())
		{
			DefaultMappingContext = IMC.Object;
		}
	}

	// Click/Touch actions (from template)
	if (!SetDestinationClickAction)
	{
		static ConstructorHelpers::FObjectFinder<UInputAction> IA_Click(TEXT("/Game/TopDown/Input/Actions/IA_SetDestination_Click.IA_SetDestination_Click"));
		if (IA_Click.Succeeded())
		{
			SetDestinationClickAction = IA_Click.Object;
		}
	}
	if (!SetDestinationTouchAction)
	{
		static ConstructorHelpers::FObjectFinder<UInputAction> IA_Touch(TEXT("/Game/TopDown/Input/Actions/IA_SetDestination_Touch.IA_SetDestination_Touch"));
		if (IA_Touch.Succeeded())
		{
			SetDestinationTouchAction = IA_Touch.Object;
		}
	}

	// New actions (WASD move & attack)
	if (!MoveAction)
	{
		static ConstructorHelpers::FObjectFinder<UInputAction> IA_Move(TEXT("/Game/TopDown/Input/Actions/IA_Move.IA_Move"));
		if (IA_Move.Succeeded())
		{
			MoveAction = IA_Move.Object;
		}
	}
	if (!AttackAction)
	{
		static ConstructorHelpers::FObjectFinder<UInputAction> IA_Attack(TEXT("/Game/TopDown/Input/Actions/IA_Attack.IA_Attack"));
		if (IA_Attack.Succeeded())
		{
			AttackAction = IA_Attack.Object;
		}
	}
}

void ATacticsPlayerController::SetupInputComponent()
{
	UE_LOG(LogTactics, Warning, TEXT("SetupInputComponent called for %s"), *GetNameSafe(this));
	
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// Only set up input on local player controllers
	if (IsLocalPlayerController())
	{
		UE_LOG(LogTactics, Warning, TEXT("Is Local Player Controller - setting up input"));
		UE_LOG(LogTactics, Warning, TEXT("DefaultMappingContext: %s"), DefaultMappingContext ? TEXT("Valid") : TEXT("NULL"));
		UE_LOG(LogTactics, Warning, TEXT("AttackAction: %s"), AttackAction ? TEXT("Valid") : TEXT("NULL"));
		// Add Input Mapping Context (if available)
		if (DefaultMappingContext)
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}

		// Set up action bindings
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
		{
			// Setup mouse input events (if actions are valid)
			if (SetDestinationClickAction)
			{
				EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &ATacticsPlayerController::OnInputStarted);
				EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &ATacticsPlayerController::OnSetDestinationTriggered);
				EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &ATacticsPlayerController::OnSetDestinationReleased);
				EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Canceled, this, &ATacticsPlayerController::OnSetDestinationReleased);
			}

			// Setup touch input events (if action is valid)
			if (SetDestinationTouchAction)
			{
				EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Started, this, &ATacticsPlayerController::OnInputStarted);
				EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Triggered, this, &ATacticsPlayerController::OnTouchTriggered);
				EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Completed, this, &ATacticsPlayerController::OnTouchReleased);
				EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Canceled, this, &ATacticsPlayerController::OnTouchReleased);
			}

			// Setup WASD movement (if action is valid)
			if (MoveAction)
			{
				EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATacticsPlayerController::OnMoveTriggered);
			}

			// Setup attack (if action is valid)
			if (AttackAction)
			{
				UE_LOG(LogTactics, Warning, TEXT("Binding IA_Attack to OnAttackTriggered"));
				EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ATacticsPlayerController::OnAttackTriggered);
			}
			else
			{
				UE_LOG(LogTactics, Error, TEXT("AttackAction is null! Cannot bind attack input."));
			}
		}
		else
		{
			UE_LOG(LogTactics, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
		}
	}
}

void ATacticsPlayerController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTactics, Warning, TEXT("PlayerController BeginPlay called: %s"), *GetNameSafe(this));
	UE_LOG(LogTactics, Warning, TEXT("Current Pawn: %s"), *GetNameSafe(GetPawn()));
}

void ATacticsPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	UE_LOG(LogTactics, Warning, TEXT("PlayerController OnPossess called with pawn: %s"), *GetNameSafe(InPawn));
	UE_LOG(LogTactics, Warning, TEXT("Pawn class: %s"), InPawn ? *InPawn->GetClass()->GetName() : TEXT("NULL"));
}

void ATacticsPlayerController::OnInputStarted()
{
	StopMovement();
}

void ATacticsPlayerController::OnSetDestinationTriggered()
{
	// We flag that the input is being pressed
	FollowTime += GetWorld()->GetDeltaSeconds();
	
	// We look for the location in the world where the player has pressed the input
	FHitResult Hit;
	bool bHitSuccessful = false;
	if (bIsTouch)
	{
		bHitSuccessful = GetHitResultUnderFinger(ETouchIndex::Touch1, ECollisionChannel::ECC_Visibility, true, Hit);
	}
	else
	{
		bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
	}

	// If we hit a surface, cache the location
	if (bHitSuccessful)
	{
		CachedDestination = Hit.Location;
	}
	
	// Move towards mouse pointer or touch
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
	}
}

void ATacticsPlayerController::OnSetDestinationReleased()
{
	// If it was a short press
	if (FollowTime <= ShortPressThreshold)
	{
		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}

	FollowTime = 0.f;
}

// Triggered every frame when the input is held down
void ATacticsPlayerController::OnTouchTriggered()
{
	bIsTouch = true;
	OnSetDestinationTriggered();
}

void ATacticsPlayerController::OnTouchReleased()
{
	bIsTouch = false;
	OnSetDestinationReleased();
}

void ATacticsPlayerController::OnMoveTriggered(const FInputActionValue& Value)
{
	// Get the 2D input value (WASD)
	const FVector2D MovementVector = Value.Get<FVector2D>();

	// Get the controlled pawn
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		// Calculate movement direction based on camera rotation
		const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
		
		// Forward/backward direction
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		ControlledPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
		
		// Right/left direction
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		ControlledPawn->AddMovementInput(RightDirection, MovementVector.X);

		// Update rotation to face mouse cursor
		UpdateCharacterRotation();
	}
}

void ATacticsPlayerController::OnAttackTriggered()
{
	UE_LOG(LogTactics, Warning, TEXT("OnAttackTriggered called!"));
	
	// Get the controlled character
	APawn* CurrentPawn = GetPawn();
	if (!CurrentPawn)
	{
		UE_LOG(LogTactics, Error, TEXT("No pawn possessed!"));
		return;
	}
	
	UE_LOG(LogTactics, Warning, TEXT("Current pawn class: %s"), *CurrentPawn->GetClass()->GetName());
	
	if (ATacticsCharacter* ControlledCharacter = Cast<ATacticsCharacter>(CurrentPawn))
	{
		UE_LOG(LogTactics, Warning, TEXT("Cast to ATacticsCharacter successful!"));
		// Perform attack
		ControlledCharacter->PerformAttack();
	}
	else
	{
		UE_LOG(LogTactics, Error, TEXT("Cast to ATacticsCharacter failed! Pawn class: %s"), *CurrentPawn->GetClass()->GetName());
	}
}

void ATacticsPlayerController::UpdateCharacterRotation()
{
	// Get hit result under cursor
	FHitResult Hit;
	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);

	if (APawn* ControlledPawn = GetPawn())
	{
		// Calculate direction from character to cursor
		const FVector Direction = Hit.Location - ControlledPawn->GetActorLocation();
		const FRotator NewRotation = FRotator(0.0f, Direction.Rotation().Yaw, 0.0f);
		
		// Smoothly rotate character
		ControlledPawn->SetActorRotation(NewRotation);
	}
}

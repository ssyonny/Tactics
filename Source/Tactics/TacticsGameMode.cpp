// Copyright Epic Games, Inc. All Rights Reserved.

#include "TacticsGameMode.h"
#include "TacticsPlayerController.h"
#include "TacticsCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATacticsGameMode::ATacticsGameMode()
{
	// Set defaults so the game runs without Blueprint overrides
	PlayerControllerClass = ATacticsPlayerController::StaticClass();
	
	// Try to load BP_TacticsCharacter first, fallback to C++ class
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDown/Blueprints/BP_TacticsCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
		UE_LOG(LogTemp, Warning, TEXT("Using BP_TacticsCharacter as default pawn from TopDown/Blueprints"));
	}
	else
	{
		// Try alternative path
		static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass2(TEXT("/Game/Characters/BP_TacticsCharacter"));
		if (PlayerPawnBPClass2.Class != nullptr)
		{
			DefaultPawnClass = PlayerPawnBPClass2.Class;
			UE_LOG(LogTemp, Warning, TEXT("Using BP_TacticsCharacter as default pawn from Characters"));
		}
		else
		{
			DefaultPawnClass = ATacticsCharacter::StaticClass();
			UE_LOG(LogTemp, Warning, TEXT("Using C++ TacticsCharacter as default pawn"));
		}
	}
}
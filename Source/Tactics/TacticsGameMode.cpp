// Copyright Epic Games, Inc. All Rights Reserved.

#include "TacticsGameMode.h"
#include "TacticsPlayerController.h"
#include "TacticsCharacter.h"

ATacticsGameMode::ATacticsGameMode()
{
	// Set defaults so the game runs without Blueprint overrides
	PlayerControllerClass = ATacticsPlayerController::StaticClass();
	DefaultPawnClass = ATacticsCharacter::StaticClass();
}
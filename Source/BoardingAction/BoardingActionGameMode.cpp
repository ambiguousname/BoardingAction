// Copyright Epic Games, Inc. All Rights Reserved.

#include "BoardingActionGameMode.h"
#include "BoardingActionHUD.h"
#include "BoardingActionCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABoardingActionGameMode::ABoardingActionGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ABoardingActionHUD::StaticClass();
}
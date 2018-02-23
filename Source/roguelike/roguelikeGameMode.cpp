// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "roguelikeGameMode.h"
#include "roguelikePlayerController.h"
#include "roguelikeCharacter.h"
#include "UObject/ConstructorHelpers.h"

AroguelikeGameMode::AroguelikeGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AroguelikePlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
// Copyright Epic Games, Inc. All Rights Reserved.

#include "AmaruShooterGameMode.h"
#include "AmaruShooterCharacter.h"
#include "UObject/ConstructorHelpers.h"

AAmaruShooterGameMode::AAmaruShooterGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modulo_1_consegaGameMode.h"
#include "Modulo_1_consegaCharacter.h"
#include "UObject/ConstructorHelpers.h"

AModulo_1_consegaGameMode::AModulo_1_consegaGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "GravityGunPlaygroundGameMode.h"
#include "GravityGunPlaygroundHUD.h"
#include "GravityGunPlaygroundCharacter.h"
#include "UObject/ConstructorHelpers.h"

AGravityGunPlaygroundGameMode::AGravityGunPlaygroundGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/External/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AGravityGunPlaygroundHUD::StaticClass();
}

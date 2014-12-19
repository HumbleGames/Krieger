// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "Krieger.h"

AKriegerGameMode::AKriegerGameMode(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	PlayerControllerClass = AKriegerPlayerController::StaticClass();
}



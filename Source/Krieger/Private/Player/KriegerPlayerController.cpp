// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "Krieger.h"

AKriegerPlayerController::AKriegerPlayerController(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{

}

void AKriegerPlayerController::InitInputSystem()
{
	Super::InitInputSystem();

	if (PlayerInput != NULL)
	{
		UKriegerUserSettings *UserSettings = CastChecked<UKriegerUserSettings>(GEngine->GetGameUserSettings());
		UserSettings->ApplySettings();
	}
}

void AKriegerPlayerController::SetGraphicsQuality(int32 InGraphicsQuality)
{
	if (PlayerInput != NULL)
	{
		UKriegerUserSettings *UserSettings = CastChecked<UKriegerUserSettings>(GEngine->GetGameUserSettings());
		UserSettings->SetGraphicsQuality(InGraphicsQuality);
		UserSettings->ApplySettings();
	}
}

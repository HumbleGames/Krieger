// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "Krieger.h"

AKriegerGameState::AKriegerGameState(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	NumTeams = 0;
}

void AKriegerGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AKriegerGameState, NumTeams);
	DOREPLIFETIME(AKriegerGameState, TeamScores);
}

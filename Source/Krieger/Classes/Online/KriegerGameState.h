// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "KriegerGameState.generated.h"

UCLASS()
class AKriegerGameState : public AGameState
{
	GENERATED_UCLASS_BODY()

	/** Number of teams in current game */
	UPROPERTY(Transient, Replicated)
	int32 NumTeams;

	/** Accumulated score per team */
	UPROPERTY(Transient, Replicated)
	TArray<int32> TeamScores;

};

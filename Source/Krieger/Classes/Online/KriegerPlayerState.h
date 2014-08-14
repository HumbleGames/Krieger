// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "KriegerPlayerState.generated.h"

UCLASS()
class AKriegerPlayerState : public APlayerState
{
	GENERATED_UCLASS_BODY()

	// Begin APlayerState interface
	/** Clear scores */
	virtual void Reset() override;

	/** Set the team */
	virtual void ClientInitialize(class AController* InController) override;
	// End APlayerState interface


	//////////////////////////////////////////////////////////////////////////
	// Teams

	/** Get current team */
	int32 GetTeamNum() const;

	/**
	 * Set new team and update pawn. Also updates player character team colors
	 *
	 * @param	NewTeamNumber	Team we want to be on
	 */
	void SetTeamNum(int32 NewTeamNumber);

	/** Replicate team colors. Updated the players mesh colors appropriately */
	UFUNCTION()
	void OnRep_TeamColor();

protected:
	/** Set the mesh colors based on the current teamnum variable */
	void UpdateTeamColors();

	/** Team number */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_TeamColor)
	int32 TeamNumber;
	
};

// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "Krieger.h"

AKriegerPlayerState::AKriegerPlayerState(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	TeamNumber = 0;
}

void AKriegerPlayerState::Reset()
{
	Super::Reset();

	SetTeamNum(0);
}

void AKriegerPlayerState::ClientInitialize(class AController* InController)
{
	Super::ClientInitialize(InController);

	UpdateTeamColors();
}


//////////////////////////////////////////////////////////////////////////
// Teams

int32 AKriegerPlayerState::GetTeamNum() const
{
	return TeamNumber;
}

void AKriegerPlayerState::SetTeamNum(int32 NewTeamNumber)
{
	TeamNumber = NewTeamNumber;

	UpdateTeamColors();
}

void AKriegerPlayerState::OnRep_TeamColor()
{
	UpdateTeamColors();
}

void AKriegerPlayerState::UpdateTeamColors()
{
	AController* OwnerController = Cast<AController>(GetOwner());
	if (OwnerController != NULL)
	{
		AKriegerCharacter* KriegerCharacter = Cast<AKriegerCharacter>(OwnerController->GetCharacter());
		if (KriegerCharacter != NULL)
		{
			KriegerCharacter->UpdateTeamColorsAllMIDs();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// Replication

void AKriegerPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AKriegerPlayerState, TeamNumber);
}

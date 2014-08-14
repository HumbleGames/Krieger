// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "Krieger.h"

AKriegerArm::AKriegerArm(const class FPostConstructInitializeProperties& PCIP) : Super(PCIP)
{
	
}

//////////////////////////////////////////////////////////////////////////
// Inventory

void AKriegerArm::AttachMeshToPawn()
{
	if (MyPawn)
	{
		// Be sure we removed prevous one result
		DetachMeshFromPawn();

		USkeletalMeshComponent* PawnMesh = MyPawn->GetPawnMesh();
		Mesh->AttachTo(PawnMesh, NAME_None, EAttachLocation::SnapToTarget);
		Mesh->SetMasterPoseComponent(PawnMesh);
		Mesh->SetHiddenInGame(false);
	}
}

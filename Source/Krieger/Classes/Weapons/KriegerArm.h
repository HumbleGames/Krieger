// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "KriegerArm.generated.h"

UCLASS(Abstract, Blueprintable)
class AKriegerArm : public AKriegerWeapon
{
	GENERATED_UCLASS_BODY()


	//////////////////////////////////////////////////////////////////////////
	// Mesh

protected:
	/** Attach arm mesh to pawn's mesh */
	virtual void AttachMeshToPawn() override;

};

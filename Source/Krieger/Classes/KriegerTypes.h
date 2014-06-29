// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "KriegerTypes.generated.h"

#pragma once

UENUM(BlueprintType)
namespace EKriegerPhysMaterialType
{
	enum Type
	{
		Unknown,
		Concrete,
		Dirt,
		Water,
		Metal,
		Wood,
		Grass,
		Glass,
		Flesh,
	};
}

#define KRIEGER_SURFACE_Default		SurfaceType_Default
#define KRIEGER_SURFACE_Concrete	SurfaceType1
#define KRIEGER_SURFACE_Dirt		SurfaceType2
#define KRIEGER_SURFACE_Water		SurfaceType3
#define KRIEGER_SURFACE_Metal		SurfaceType4
#define KRIEGER_SURFACE_Wood		SurfaceType5
#define KRIEGER_SURFACE_Grass		SurfaceType6
#define KRIEGER_SURFACE_Glass		SurfaceType7
#define KRIEGER_SURFACE_Flesh		SurfaceType8

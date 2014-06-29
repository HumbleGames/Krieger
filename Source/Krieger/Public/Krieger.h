// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#pragma once

#include "Engine.h"
#include "Net/UnrealNetwork.h"

#include "KriegerClasses.h"

/** When you modify this, please note that this information can be saved with instances
 * also DefaultEngine.ini [/Script/Engine.CollisionProfile] should match with this list **/
#define COLLISION_WEAPON		ECC_GameTraceChannel1
#define COLLISION_PROJECTILE	ECC_GameTraceChannel2
#define COLLISION_PICKUP		ECC_GameTraceChannel3

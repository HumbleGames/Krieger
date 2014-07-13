// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "Krieger.h"
#include "Krieger.generated.inl"

class FKriegerGameModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() OVERRIDE
	{

	}

	virtual void ShutdownModule() OVERRIDE
	{

	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FKriegerGameModule, Krieger, "Krieger");

DEFINE_LOG_CATEGORY(LogKrieger)
DEFINE_LOG_CATEGORY(LogWeapon)

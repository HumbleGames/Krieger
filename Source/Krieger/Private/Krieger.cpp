// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "Krieger.h"

class FKriegerGameModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{

	}

	virtual void ShutdownModule() override
	{

	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FKriegerGameModule, Krieger, "Krieger");

DEFINE_LOG_CATEGORY(LogKrieger)
DEFINE_LOG_CATEGORY(LogWeapon)

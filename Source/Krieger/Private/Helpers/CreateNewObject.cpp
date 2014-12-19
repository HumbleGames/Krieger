// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

#include "Krieger.h"

UCreateNewObject::UCreateNewObject(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{

}

UObject* UCreateNewObject::NewObjectFromBlueprint(UObject* WorldContextObject, TSubclassOf<UObject> UC)
{
	UObject* tempObject = StaticConstructObject(UC);

	return tempObject;
}

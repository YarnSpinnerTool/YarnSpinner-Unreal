// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FStaticPartyModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
    
    

};

class STATICPARTY_API StaticPartyMethods {
public:
    static int HelloFromStaticParty();
    static FString GimmeSomeJSON();
};


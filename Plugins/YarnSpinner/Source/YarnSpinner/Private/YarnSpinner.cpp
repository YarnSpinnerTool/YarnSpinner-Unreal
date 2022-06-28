// Copyright Epic Games, Inc. All Rights Reserved.

#include "YarnSpinner.h"

#define LOCTEXT_NAMESPACE "FYarnSpinnerModule"

void FYarnSpinnerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FYarnSpinnerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

bool FYarnSpinnerModule::SupportsDynamicReloading()
{
    return true;
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FYarnSpinnerModule, YarnSpinner)
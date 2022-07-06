// Copyright Epic Games, Inc. All Rights Reserved.

#include "YarnSpinner.h"
#include <google/protobuf/stubs/logging.h>

#define LOCTEXT_NAMESPACE "FYarnSpinnerModule"

DEFINE_LOG_CATEGORY(LogYarnSpinner);

void UnrealLogHandler(google::protobuf::LogLevel level, const char* filename, int line,
                        const std::string& message) {
	UE_LOG(LogYarnSpinner, Warning, TEXT("Protobuf: %s"), message.c_str());
}


void FYarnSpinnerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	SetLogHandler(UnrealLogHandler);
	UE_LOG(LogYarnSpinner, Warning, TEXT("Installed Protobuf log handler"));
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

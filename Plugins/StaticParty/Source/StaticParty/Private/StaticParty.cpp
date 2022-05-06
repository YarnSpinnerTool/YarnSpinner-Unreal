// Copyright Epic Games, Inc. All Rights Reserved.

#include "StaticParty.h"
#include "yarn_spinner.pb.h"

THIRD_PARTY_INCLUDES_START
#include <string>
#include <google/protobuf/util/json_util.h>
THIRD_PARTY_INCLUDES_END

#define LOCTEXT_NAMESPACE "FStaticPartyModule"

void FStaticPartyModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
    UE_LOG(LogTemp, Warning, TEXT("Hello from StaticParty"));
}

void FStaticPartyModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
    
    // Unload the protobuf library before shutting down.
    google::protobuf::ShutdownProtobufLibrary();
}

FString StaticPartyMethods::GimmeSomeJSON() {
    auto node = new Yarn::Node();
    node->set_name("Hello!");
    
    auto string = new std::string();
    
    google::protobuf::util::MessageToJsonString(*node, string);
    
    return FString(string->c_str());
}

int StaticPartyMethods::HelloFromStaticParty() {
    return 1338;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FStaticPartyModule, StaticParty)

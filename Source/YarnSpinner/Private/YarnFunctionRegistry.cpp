// Fill out your copyright notice in the Description page of Project Settings.


#include "YarnFunctionRegistry.h"

#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "Misc/YarnAssetHelpers.h"
#include "Misc/YSLogging.h"


UYarnFunctionRegistry::UYarnFunctionRegistry()
{
    YS_LOG_FUNCSIG
    const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    // Set up asset registry delegates
    {
        OnAssetRegistryFilesLoadedHandle = AssetRegistry.OnFilesLoaded().AddUObject(this, &UYarnFunctionRegistry::OnAssetRegistryFilesLoaded);
        OnAssetAddedHandle = AssetRegistry.OnAssetAdded().AddUObject(this, &UYarnFunctionRegistry::OnAssetAdded);
        OnAssetRemovedHandle = AssetRegistry.OnAssetRemoved().AddUObject(this, &UYarnFunctionRegistry::OnAssetRemoved);
        OnAssetRenamedHandle = AssetRegistry.OnAssetRenamed().AddUObject(this, &UYarnFunctionRegistry::OnAssetRenamed);
    }
}


void UYarnFunctionRegistry::BeginDestroy()
{
    YS_LOG_FUNCSIG
    UObject::BeginDestroy();

    IAssetRegistry& AssetRegistry = FAssetRegistryModule::GetRegistry();
    AssetRegistry.OnFilesLoaded().Remove(OnAssetRegistryFilesLoadedHandle);
    AssetRegistry.OnAssetAdded().Remove(OnAssetAddedHandle);
    AssetRegistry.OnAssetRemoved().Remove(OnAssetRemovedHandle);
    AssetRegistry.OnAssetRenamed().Remove(OnAssetRenamedHandle);
}


void UYarnFunctionRegistry::FindFunctions()
{
    YS_LOG_FUNCSIG
    YS_LOG("Project content dir: %s", *FPaths::ProjectContentDir());

    // TODO: also check levels for level blueprints (and maybe prefix the level name to the function name?)
    // TArray<FAssetData> ExistingAssets = FYarnAssetHelpers::FindAssetsInRegistry<UBlueprint>();
    TArray<FAssetData> ExistingAssets = FYarnAssetHelpers::FindAssetsInRegistryByPackagePath<UBlueprint>(FPaths::GetPath("/Game/"));
    // TArray<FAssetData> ExistingAssets = FYarnAssetHelpers::FindAssetsInRegistry<UBlueprintFunctionLibrary>();

    for (auto Asset : ExistingAssets)
    {
        YS_LOG_FUNC("Found asset: %s (%s) %s", *Asset.GetFullName(), *Asset.GetPackage()->GetName(), *Asset.GetAsset()->GetPathName())
        auto Lib = Cast<UBlueprint>(Asset.GetAsset());
        // Lib->CallFunctionByNameWithArguments(TEXT("DoStuff"))
        // Lib->
        // YS_LOG("Supports functions: %d", Lib->SupportsFunctions())
        for (auto Func : Lib->FunctionGraphs)
        {
            YS_LOG("Function graph: %s", *Func->GetName())
            for (auto Node : Func->Nodes)
            {
                // Simple 3 node function (entry, exit, and a call to a comparitor)
                // Node: K2Node_FunctionEntry_0(Class: K2Node_FunctionEntry, Title: My Lib Function)
                // Node: K2Node_FunctionResult_0(Class: K2Node_FunctionResult, Title: Return Node)
                // Node: K2Node_CallFunction_0(Class: K2Node_CallFunction, Title: integer > integer)
                YS_LOG("Node: %s (Class: %s, Title: %s)", *Node->GetName(), *Node->GetClass()->GetName(), *Node->GetNodeTitle(ENodeTitleType::ListView).ToString())
                if (Node->IsA(UK2Node_FunctionEntry::StaticClass()))
                {
                    YS_LOG("Node is a function entry node with pins: %d", Node->Pins.Num())
                    for (auto Pin : Node->Pins)
                    {
                        YS_LOG("Pin: %s (Direction: %d, PinType: %s)", *Pin->GetName(), (int)Pin->Direction, *Pin->PinType.PinCategory.ToString())
                    }
                }
                else if (Node->IsA(UK2Node_FunctionResult::StaticClass()))
                {
                    YS_LOG("Node is a function result node with pins: %d", Node->Pins.Num())
                }
                else if (Node->IsA(UK2Node_FunctionTerminator::StaticClass()))
                {
                    YS_LOG("Node is a function terminator node with pins: %d", Node->Pins.Num())
                }
            }
        }
    }
}


void UYarnFunctionRegistry::OnAssetRegistryFilesLoaded()
{
    YS_LOG_FUNCSIG
    bRegistryFilesLoaded = true;
    FindFunctions();
}


void UYarnFunctionRegistry::OnAssetAdded(const FAssetData& AssetData)
{
    // Ignore this until the registry has finished loading the first time.
    if (!bRegistryFilesLoaded)
        return;
    YS_LOG_FUNCSIG
}


void UYarnFunctionRegistry::OnAssetRemoved(const FAssetData& AssetData)
{
    YS_LOG_FUNCSIG
}


void UYarnFunctionRegistry::OnAssetRenamed(const FAssetData& AssetData, const FString& String)
{
    YS_LOG_FUNCSIG
}

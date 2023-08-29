// Fill out your copyright notice in the Description page of Project Settings.


#include "YarnEditorSubsystem.h"

#include "AssetToolsModule.h"
#include "FindInBlueprintManager.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "YarnFunctionLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Misc/YarnAssetHelpers.h"
#include "Misc/YSLogging.h"


void UYarnEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    YS_LOG_FUNCSIG
    Super::Initialize(Collection);

    YarnFunctionRegistry = NewObject<UYarnFunctionLibrary>(this, "YarnFunctionRegistry");


    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

    const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    // Set up asset registry delegates
    {
        OnAssetRegistryFilesLoadedHandle = AssetRegistry.OnFilesLoaded().AddUObject(this, &UYarnEditorSubsystem::OnAssetRegistryFilesLoaded);
        OnAssetAddedHandle = AssetRegistry.OnAssetAdded().AddUObject(this, &UYarnEditorSubsystem::OnAssetAdded);
        OnAssetRemovedHandle = AssetRegistry.OnAssetRemoved().AddUObject(this, &UYarnEditorSubsystem::OnAssetRemoved);
        OnAssetRenamedHandle = AssetRegistry.OnAssetRenamed().AddUObject(this, &UYarnEditorSubsystem::OnAssetRenamed);
    }
    return;


    // FindFunction() // find a UFUNCTION() by name

    // Can't be used from here
    // auto BlueprintSearchManager = FFindInBlueprintSearchManager::Get();
    // This seems to exist at this point but if it ever doesn't we might need to either wait or trigger the creation some other way
    BlueprintSearchManager = FFindInBlueprintSearchManager::Instance;

    if (!BlueprintSearchManager)
    {
        YS_WARN("No BlueprintSearchManager found.")
        return;
    }
    YS_LOG("BlueprintSearchManager found.")

    if (BlueprintSearchManager->IsAssetDiscoveryInProgress() || BlueprintSearchManager->IsCacheInProgress() || BlueprintSearchManager->IsUnindexedCacheInProgress() || BlueprintSearchManager->GetCacheProgress() < 1)
    {
        YS_WARN("Can't search, still caching")
        return;
    }
    YS_LOG("Cache available, will search.")


    auto Data = BlueprintSearchManager->GetSearchDataForAssetPath(FName("/Game"));

    YS_LOG("Search data: %s", *Data.Value)


    return;

    FStreamSearchOptions Options;
    Options.ImaginaryDataFilter = ESearchQueryFilter::FunctionsFilter;
    FStreamSearch Search("NewFunction_0", Options);
    YS_LOG("About to search.")


    BlueprintSearchManager->BeginSearchQuery(&Search);
    YS_LOG("Query begun.")

    FSearchData SearchData;
    while (BlueprintSearchManager->ContinueSearchQuery(&Search, SearchData))
    {
        YS_LOG("Found a function: %s", *SearchData.Blueprint->GetName())
    }
    YS_LOG("Finished, ensuring end.")
    BlueprintSearchManager->EnsureSearchQueryEnds(&Search);
    YS_LOG("ALLL DDDDONNNNEE.")

    // if (FFindInBlueprintSearchManager::Instance)
    //     YS_LOG("GOT AN INSTANCE")
    // else
    // {
    //     YS_LOG("NO INSTANCEEEEEEE")
    // }
    //
    // if (GetWorld())
    //     YS_LOG("We have a world")
    // else
    // {
    //     YS_LOG("No world :(")
    // }
}


void UYarnEditorSubsystem::Deinitialize()
{
    YS_LOG_FUNCSIG
    Super::Deinitialize();
}




void UYarnEditorSubsystem::OnAssetRegistryFilesLoaded()
{
}


void UYarnEditorSubsystem::OnAssetAdded(const FAssetData& AssetData) const
{
}


void UYarnEditorSubsystem::OnAssetRemoved(const FAssetData& AssetData) const
{
}


void UYarnEditorSubsystem::OnAssetRenamed(const FAssetData& AssetData, const FString& OldObjectPath) const
{
}

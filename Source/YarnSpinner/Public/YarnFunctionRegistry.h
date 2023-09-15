// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "UObject/Object.h"
#include "YarnSpinnerCore/Value.h"
#include "YarnFunctionRegistry.generated.h"


USTRUCT()
struct FYarnBPFuncParam
{
    GENERATED_BODY()

    FName Name;
    
    Yarn::Value Value;
};


USTRUCT()
struct FYarnBPLibFunction
{
    GENERATED_BODY()

    UBlueprint* Library;

    FName Name;

    TArray<FYarnBPFuncParam> InParams;
    TOptional<FYarnBPFuncParam> OutParam;
};


/**
 * 
 */
UCLASS()
class YARNSPINNER_API UYarnFunctionRegistry : public UObject
{
    GENERATED_BODY()

public:
    UYarnFunctionRegistry();
    virtual void BeginDestroy() override;
    
private:
    // Blueprints that extend YarnFunctionLibrary
    UPROPERTY()
    TSet<UBlueprint*> Libraries;

    // A map of blueprints to a list of their function names
    TMap<UBlueprint*, TArray<FName>> LibFunctions;
    // A map of function names to lists of details of implementations
    TMap<FName, TArray<FYarnBPLibFunction>> AllFunctions;
    
    FDelegateHandle OnAssetRegistryFilesLoadedHandle;
    FDelegateHandle OnAssetAddedHandle;
    FDelegateHandle OnAssetRemovedHandle;
    FDelegateHandle OnAssetUpdatedHandle;
    FDelegateHandle OnAssetRenamedHandle;
    bool bRegistryFilesLoaded = false;

    UBlueprint* GetYarnFunctionLibraryBlueprint(const FAssetData& AssetData) const;
    void FindFunctions();
    // Import functions for a given Blueprint
    void ImportFunctions(UBlueprint* YarnFunctionLibrary);
    // Update functions for a given Blueprint
    void UpdateFunctions(UBlueprint* YarnFunctionLibrary);
    // Clear functions and references for a given Blueprint
    void RemoveFunctions(UBlueprint* YarnFunctionLibrary);
    
    void OnAssetRegistryFilesLoaded();
    void OnAssetAdded(const FAssetData& AssetData);
    void OnAssetRemoved(const FAssetData& AssetData);
    void OnAssetUpdated(const FAssetData& AssetData);
    void OnAssetRenamed(const FAssetData& AssetData, const FString& String);
    void OnStartGameInstance(UGameInstance* GameInstance);
};

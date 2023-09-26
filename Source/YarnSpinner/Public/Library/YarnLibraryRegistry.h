// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "UObject/Object.h"
#include "YarnSpinnerCore/Value.h"
#include "YarnLibraryRegistry.generated.h"


USTRUCT()
struct FYarnBlueprintFuncParam
{
    GENERATED_BODY()

    FName Name;
    
    Yarn::Value Value;
};


USTRUCT()
struct FYarnBlueprintLibFunction
{
    GENERATED_BODY()

    UBlueprint* Library;
    FName Name;

    TArray<FYarnBlueprintFuncParam> InParams;
    TOptional<FYarnBlueprintFuncParam> OutParam;
};


USTRUCT()
struct FYarnBlueprintLibFunctionMeta
{
    GENERATED_BODY()

    bool bIsPublic = false;
    bool bIsPure = false;
    bool bIsConst = false;
    bool bHasMultipleOutParams = false;
    // In/out parameters with invalid types
    TArray<FString> InvalidParams;
};


/**
 * 
 */
UCLASS()
class YARNSPINNER_API UYarnLibraryRegistry : public UObject
{
    GENERATED_BODY()

public:
    UYarnLibraryRegistry();
    virtual void BeginDestroy() override;

    const TMap<FName, FYarnBlueprintLibFunction>& GetFunctions() const;
    const TMap<FName, FYarnBlueprintLibFunction>& GetCommands() const;
    bool HasFunction(const FName& Name) const;
    int GetExpectedFunctionParamCount(const FName& Name) const;
    Yarn::Value CallFunction(const FName& Name, TArray<Yarn::Value> Parameters) const;

private:
    // Blueprints that extend YarnFunctionLibrary
    UPROPERTY()
    TSet<UBlueprint*> FunctionLibraries;
    UPROPERTY()
    TSet<UBlueprint*> CommandLibraries;

    // A map of blueprints to a list of their function names
    TMap<UBlueprint*, TArray<FName>> LibFunctions;
    TMap<UBlueprint*, TArray<FName>> LibCommands;
    // A map of function names to lists of details of implementations
    TMap<FName, FYarnBlueprintLibFunction> AllFunctions;
    TMap<FName, FYarnBlueprintLibFunction> AllCommands;
    
    FDelegateHandle OnAssetRegistryFilesLoadedHandle;
    FDelegateHandle OnAssetAddedHandle;
    FDelegateHandle OnAssetRemovedHandle;
    FDelegateHandle OnAssetUpdatedHandle;
    FDelegateHandle OnAssetRenamedHandle;
    bool bRegistryFilesLoaded = false;

    static UBlueprint* GetYarnFunctionLibraryBlueprint(const FAssetData& AssetData);
    static UBlueprint* GetYarnCommandLibraryBlueprint(const FAssetData& AssetData);
    void FindFunctionsAndCommands();
    static void ExtractFunctionDataFromBlueprintGraph(UBlueprint* YarnFunctionLibrary, UEdGraph* Func, FYarnBlueprintLibFunction& FuncDetails, FYarnBlueprintLibFunctionMeta& FuncMeta);
    // Import functions for a given Blueprint
    void ImportFunctions(UBlueprint* YarnFunctionLibrary);
    // Import functions for a given Blueprint
    void ImportCommands(UBlueprint* YarnCommandLibrary);
    // Update functions for a given Blueprint
    void UpdateFunctions(UBlueprint* YarnFunctionLibrary);
    // Update functions for a given Blueprint
    void UpdateCommands(UBlueprint* YarnCommandLibrary);
    // Clear functions and references for a given Blueprint
    void RemoveFunctions(UBlueprint* YarnFunctionLibrary);
    // Clear functions and references for a given Blueprint
    void RemoveCommands(UBlueprint* YarnCommandLibrary);
    
    void OnAssetRegistryFilesLoaded();
    void OnAssetAdded(const FAssetData& AssetData);
    void OnAssetRemoved(const FAssetData& AssetData);
    void OnAssetUpdated(const FAssetData& AssetData);
    void OnAssetRenamed(const FAssetData& AssetData, const FString& String);
    void OnStartGameInstance(UGameInstance* GameInstance);
};

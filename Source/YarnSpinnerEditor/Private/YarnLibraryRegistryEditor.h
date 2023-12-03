// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Library/YarnLibraryRegistry.h"
#include "Library/YarnSpinnerLibraryData.h"
#include "YarnSpinnerCore/Value.h"
#include "UObject/Object.h"
#include "YarnLibraryRegistryEditor.generated.h"


/**
 * 
 */
UCLASS()
class YARNSPINNEREDITOR_API UYarnLibraryRegistryEditor : public UObject
{
    GENERATED_BODY()

public:
    UYarnLibraryRegistryEditor();
    virtual void BeginDestroy() override;

private:
    FYarnSpinnerLibraryData YSLSData;
    
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
    TMap<FName, FYarnStdLibFunction> StdFunctions;
    TMap<FName, FYarnBlueprintLibFunction> AllCommands;

    FDelegateHandle OnAssetRegistryFilesLoadedHandle;
    FDelegateHandle OnAssetAddedHandle;
    FDelegateHandle OnAssetRemovedHandle;
    FDelegateHandle OnAssetUpdatedHandle;
    FDelegateHandle OnAssetRenamedHandle;
    bool bRegistryEditorFilesLoaded = false;
    
    static UBlueprint* GetYarnFunctionLibraryBlueprint(const FAssetData& AssetData);
    static UBlueprint* GetYarnCommandLibraryBlueprint(const FAssetData& AssetData);
    void SaveYSLS();
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

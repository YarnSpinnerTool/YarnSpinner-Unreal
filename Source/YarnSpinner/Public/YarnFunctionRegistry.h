// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "UObject/Object.h"
#include "YarnFunctionRegistry.generated.h"

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
    
    void FindFunctions();
    
private:
    FDelegateHandle OnAssetRegistryFilesLoadedHandle;
    FDelegateHandle OnAssetAddedHandle;
    FDelegateHandle OnAssetRemovedHandle;
    FDelegateHandle OnAssetRenamedHandle;
    bool bRegistryFilesLoaded = false;

    void OnAssetRegistryFilesLoaded();
    void OnAssetAdded(const FAssetData& AssetData);
    void OnAssetRemoved(const FAssetData& AssetData);
    void OnAssetRenamed(const FAssetData& AssetData, const FString& String);
    void OnStartGameInstance(UGameInstance* GameInstance);
};

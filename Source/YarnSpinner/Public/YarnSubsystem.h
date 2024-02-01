// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Library/YarnLibraryRegistry.h"
#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include "YarnProject.h"
#include "Engine/DataTable.h"
#include "Engine/ObjectLibrary.h"
#include "YarnSpinnerCore/VirtualMachine.h"

#include "YarnSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class YARNSPINNER_API UYarnSubsystem : public UGameInstanceSubsystem, public Yarn::IVariableStorage
{
    GENERATED_BODY()
public:
    UYarnSubsystem();
    
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    virtual void SetValue(const FString& name, bool value);
    virtual void SetValue(const FString& name, float value);
    virtual void SetValue(const FString& name, const FString& value);

    virtual bool HasValue(const FString& name);
    virtual Yarn::FValue GetValue(const FString& name);

    virtual void ClearValue(const FString& name);

    const UYarnLibraryRegistry* GetYarnLibraryRegistry() const { return YarnFunctionRegistry; }

private:
    // UPROPERTY()
    // TMap<UYarnProjectAsset*, TMap<FName, UDataTable*>> LocTextDataTables;

    UPROPERTY()
    UYarnLibraryRegistry* YarnFunctionRegistry;

    UPROPERTY()
    UObjectLibrary* YarnFunctionObjectLibrary;
    UPROPERTY()
    UObjectLibrary* YarnCommandObjectLibrary;
    
    TMap<FString, Yarn::FValue> Variables;
    
    FDelegateHandle OnAssetRegistryFilesLoadedHandle;
    FDelegateHandle OnLevelAddedToWorldHandle;
    FDelegateHandle OnWorldInitializedActorsHandle;
    
};



// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include "YarnProject.h"
#include "Engine/DataTable.h"
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

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    virtual void SetValue(std::string name, bool value);
    virtual void SetValue(std::string name, float value);
    virtual void SetValue(std::string name, std::string value);

    virtual bool HasValue(std::string name);
    virtual Yarn::Value GetValue(std::string name);

    virtual void ClearValue(std::string name);

private:
    // UPROPERTY()
    // TMap<UYarnProjectAsset*, TMap<FName, UDataTable*>> LocTextDataTables;

    TMap<FString, Yarn::Value> Variables;
};



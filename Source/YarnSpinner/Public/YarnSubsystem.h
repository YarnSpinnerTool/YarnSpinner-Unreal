// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include "YarnProject.h"
#include "Engine/DataTable.h"

#include "YarnSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class YARNSPINNER_API UYarnSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    FString GetLocText(const UYarnProject* YarnProject, const FName& Language, const FName& LineID);

private:
    // UPROPERTY()
    // TMap<UYarnProjectAsset*, TMap<FName, UDataTable*>> LocTextDataTables;
    
    UPROPERTY()
    TMap<FName, UDataTable*> LocTextDataTables;
};



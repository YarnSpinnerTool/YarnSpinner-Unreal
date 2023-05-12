// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "YarnAsset.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, AutoExpandCategories = "ImportOptions")
class YARNSPINNER_API UYarnAsset : public UObject
{
	GENERATED_BODY()
    
public:
    
    UPROPERTY(VisibleAnywhere, Category="Yarn Spinner")
    TArray<uint8> Data;

    UPROPERTY(VisibleAnywhere, Category="Yarn Spinner")
    TMap<FName, FString> Lines;

#if WITH_EDITORONLY_DATA
    virtual void PostInitProperties() override;
	
    /** The file this data table was imported from, may be empty */
	UPROPERTY(VisibleAnywhere, Instanced, Category=ImportSource)
	class UAssetImportData* AssetImportData;
#endif
	
};

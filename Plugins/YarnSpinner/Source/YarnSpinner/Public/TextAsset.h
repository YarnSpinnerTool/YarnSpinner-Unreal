// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TextAsset.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, AutoExpandCategories = "ImportOptions")
class YARNSPINNER_API UTextAsset : public UObject
{
	GENERATED_BODY()
    
public:
    
    UPROPERTY(EditAnywhere, Category = "Properties")
    FString Text;
    
#if WITH_EDITORONLY_DATA
    YARNSPINNER_API virtual void PostInitProperties() override;
	
    /** The file this data table was imported from, may be empty */
	UPROPERTY(VisibleAnywhere, Instanced, Category=ImportSource)
	class UAssetImportData* AssetImportData;
#endif
	
};

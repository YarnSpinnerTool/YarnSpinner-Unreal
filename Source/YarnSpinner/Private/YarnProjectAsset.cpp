// Fill out your copyright notice in the Description page of Project Settings.


#include "YarnProjectAsset.h"

#include "YarnSpinner.h"
#include "EditorFramework/AssetImportData.h"

#if WITH_EDITORONLY_DATA
void UYarnProjectAsset::PostInitProperties() {
    if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}

	Super::PostInitProperties();
}
#endif
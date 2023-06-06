// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class YARNSPINNEREDITOR_API FYarnProjectSynchronizer
{
public:
	FYarnProjectSynchronizer();
	~FYarnProjectSynchronizer();
	
	void Setup();
	void TearDown();

private:
	FDelegateHandle OnAssetRegistryFilesLoadedHandle;
	FDelegateHandle OnAssetAddedHandle;
	FDelegateHandle OnAssetRemovedHandle;
	FDelegateHandle OnAssetRenamedHandle;
	FTimerHandle TimerHandle;

	// Callback for when the asset registry has finished scanning assets on Unreal Editor load.
	void OnAssetRegistryFilesLoaded();
	void OnAssetAdded(const FAssetData& AssetData) const;
	void OnAssetRemoved(const FAssetData& AssetData) const;
	void OnAssetRenamed(const FAssetData& AssetData, const FString& OldObjectPath) const;

	// Scan all YarnProjectAssets and update them as necessary.
	static void UpdateAllYarnProjects();
	// Checks if a yarn project's source files has changed since last compile and recompiles if necessary.
	static void UpdateYarnProjectAsset(class UYarnProjectAsset* YarnProjectAsset);
	// Reads a .yarnproject file for a YarnProjectAsset and calls localisation updaters for each locale's data.
	static void UpdateYarnProjectAssetLocalizations(const class UYarnProjectAsset* YarnProjectAsset);
	// Checks if a yarn project's localization asset files have changed since last import and imports/reimports/deletes if necessary.
	static void UpdateYarnProjectAssetLocalizationAssets(const UYarnProjectAsset* YarnProjectAsset, const FString& Loc, const FString& LocAssets);
	static void UpdateYarnProjectAssetLocalizationStrings(const UYarnProjectAsset* YarnProjectAsset, const FString& Loc, const FString& LocStrings);
};

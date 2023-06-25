// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IDirectoryWatcher.h"
#include "YarnProjectSynchronizer.generated.h"


UENUM()
enum class EYSFileAction : uint8
{
	Added, Modified, Removed, Unknown
};


/**
 * 
 */
class YARNSPINNEREDITOR_API FYarnProjectSynchronizer
{
public:
	FYarnProjectSynchronizer();
	~FYarnProjectSynchronizer();

	void OnYarnSourceDirectoryChanged(const TArray<struct FFileChangeData>& Array) const;
	void Setup();
	void TearDown();

	void SetLocFileImporter(class UCSVImportFactory* Importer);

	static EYSFileAction ToFileAction(FFileChangeData::EFileChangeAction InAction);
	
private:
	FDelegateHandle OnAssetRegistryFilesLoadedHandle;
	FDelegateHandle OnAssetAddedHandle;
	FDelegateHandle OnAssetRemovedHandle;
	FDelegateHandle OnAssetRenamedHandle;
	FTimerHandle TimerHandle;
	class UCSVImportFactory* LocFileImporter = nullptr;
	FDelegateHandle DirectoryWatcherHandle;

	// Callback for when the asset registry has finished scanning assets on Unreal Editor load.
	void OnAssetRegistryFilesLoaded();
	void OnAssetAdded(const FAssetData& AssetData) const;
	void OnAssetRemoved(const FAssetData& AssetData) const;
	void OnAssetRenamed(const FAssetData& AssetData, const FString& OldObjectPath) const;

	// Scan all YarnProjectAssets and update them as necessary.
	void UpdateAllYarnProjects() const;
	// Checks if a yarn project's source files has changed since last compile and recompiles if necessary.
	void UpdateYarnProjectAsset(class UYarnProjectAsset* YarnProjectAsset) const;
	// Reads a .yarnproject file for a YarnProjectAsset and calls localisation updaters for each locale's data.
	void UpdateYarnProjectAssetLocalizations(const class UYarnProjectAsset* YarnProjectAsset) const;

	static FString AbsoluteSourcePath(const class UYarnProjectAsset* YarnProjectAsset, const FString& SourcePath);
	// void UpdateChangedLocStrings(const UYarnProjectAsset* YarnProjectAsset, const FString& Loc, const FString& LocStrings) const;
	void UpdateLocAssets(const UYarnProjectAsset* YarnProjectAsset, const FString& Loc, const FString& LocAssets) const;
	void UpdateLocStrings(const UYarnProjectAsset* YarnProjectAsset, const FString& Loc, const FString& LocStrings) const;
	
	// Helper function for importing or updating assets of different types (voice overs, strings files, etc)
	template <class AssetClass>
	void UpdateYarnProjectAssets(const UYarnProjectAsset* YarnProjectAsset, const FString& SourcesPath, const FString& Loc, const TArray<FString>& LocSources, TFunction<TArray<UObject*>(const FString& SourceFile, const FString& DestinationPackage)> ImportNew, TFunction<bool(AssetClass* Asset)> Reimport, TSubclassOf<UObject> TheAssetClass = AssetClass::StaticClass()) const;
};

// Fill out your copyright notice in the Description page of Project Settings.


#include "YarnProjectSynchronizer.h"

#include "AssetToolsModule.h"
#include "DirectoryWatcherModule.h"
#include "FileCache.h"
#include "ObjectTools.h"
#include "YarnAssetFactory.h"
#include "YarnProjectAsset.h"
#include "YarnProjectMeta.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorFramework/AssetImportData.h"
#include "Factories/CSVImportFactory.h"
#include "Internationalization/StringTable.h"
#include "Misc/YSLogging.h"
#include "Sound/SoundWave.h"


FYarnProjectSynchronizer::FYarnProjectSynchronizer()
{
	Setup();
}


FYarnProjectSynchronizer::~FYarnProjectSynchronizer()
{
	TearDown();
}


void FYarnProjectSynchronizer::Setup()
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Set up asset registry delegates
	{
		OnAssetRegistryFilesLoadedHandle = AssetRegistry.OnFilesLoaded().AddRaw(this, &FYarnProjectSynchronizer::OnAssetRegistryFilesLoaded);
		OnAssetAddedHandle = AssetRegistry.OnAssetAdded().AddRaw(this, &FYarnProjectSynchronizer::OnAssetAdded);
		OnAssetRemovedHandle = AssetRegistry.OnAssetRemoved().AddRaw(this, &FYarnProjectSynchronizer::OnAssetRemoved);
		OnAssetRenamedHandle = AssetRegistry.OnAssetRenamed().AddRaw(this, &FYarnProjectSynchronizer::OnAssetRenamed);
	}

	// TODO: monitor yarn source folders for changes (esp. new & deleted files) -- maybe better on a per-project basis
	// FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>("DirectoryWatcher");
	// DirectoryWatcherModule.Get()->RegisterDirectoryChangedCallback_Handle(....);
}


void FYarnProjectSynchronizer::TearDown()
{
	if (const FAssetRegistryModule* AssetRegistryModule = FModuleManager::GetModulePtr<FAssetRegistryModule>("AssetRegistry"))
	{
		AssetRegistryModule->Get().OnFilesLoaded().Remove(OnAssetRegistryFilesLoadedHandle);
		OnAssetRegistryFilesLoadedHandle.Reset();
		AssetRegistryModule->Get().OnAssetAdded().Remove(OnAssetAddedHandle);
		OnAssetAddedHandle.Reset();
		AssetRegistryModule->Get().OnAssetRemoved().Remove(OnAssetRemovedHandle);
		OnAssetRemovedHandle.Reset();
		AssetRegistryModule->Get().OnAssetRenamed().Remove(OnAssetRenamedHandle);
		OnAssetRenamedHandle.Reset();
	}

	if (GEditor && GEditor->GetEditorWorldContext().World())
		GEditor->GetEditorWorldContext().World()->GetTimerManager().ClearTimer(TimerHandle);
}


void FYarnProjectSynchronizer::SetLocFileImporter(UCSVImportFactory* Importer)
{
	LocFileImporter = Importer;
}


void FYarnProjectSynchronizer::OnAssetRegistryFilesLoaded()
{
	// Set a timer to update yarn projects in a loop with a max-10-sec delay between loops.
	// TODO: consider replacing the loop with directory watching (complicated by the fact that we need to watch multiple directories per yarn project) and registry callbacks
	GEditor->GetEditorWorldContext().World()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateRaw(this, &FYarnProjectSynchronizer::UpdateAllYarnProjects), 20.0f, true);
}


void FYarnProjectSynchronizer::OnAssetAdded(const FAssetData& AssetData) const
{
	// TODO: check if asset is a yarn project; if so update its localisation assets
}


void FYarnProjectSynchronizer::OnAssetRemoved(const FAssetData& AssetData) const
{
	// TODO: check if asset is a yarn project; if so delete its localisation assets
}


void FYarnProjectSynchronizer::OnAssetRenamed(const FAssetData& AssetData, const FString& OldObjectPath) const
{
	// TODO: check if asset is a yarn project; if so move its localisation assets
}


void FYarnProjectSynchronizer::UpdateAllYarnProjects() const
{
	FARFilter YarnProjectFilter;
	YarnProjectFilter.ClassNames.Add(UYarnProjectAsset::StaticClass()->GetFName());

	TArray<FAssetData> YarnProjectAssets;
	FAssetRegistryModule::GetRegistry().GetAssets(YarnProjectFilter, YarnProjectAssets);

	for (auto Asset : YarnProjectAssets)
	{
		UYarnProjectAsset* YarnProjectAsset = Cast<UYarnProjectAsset>(Asset.GetAsset());
		YS_LOG("YarnProject Asset found: %s", *Asset.AssetName.ToString());

		UpdateYarnProjectAsset(YarnProjectAsset);
		UpdateYarnProjectAssetLocalizations(YarnProjectAsset);
	}
}


void FYarnProjectSynchronizer::UpdateYarnProjectAsset(UYarnProjectAsset* YarnProjectAsset) const
{
	YS_LOG("Checking .yarnproject asset; %s...", *YarnProjectAsset->GetName())
	// Check if yarn sources need a recompile
	TArray<FString> Sources;
	UYarnAssetFactory::GetSourcesForProject(YarnProjectAsset, Sources);
	if (YarnProjectAsset->ShouldRecompile(Sources))
	{
		YS_LOG(".yarnproject file is out of date; recompiling %s...", *YarnProjectAsset->GetName())
		if (FReimportManager::Instance()->Reimport(YarnProjectAsset, /*bAskForNewFileIfMissing=*/false))
		{
			YS_LOG("Reimport succeeded");
		}
		else
		{
			YS_WARN("Reimport failed");
		}
	}
	else
	{
		YS_LOG("Yarn project is up to date; no recompilation necessary")
	}
}


FString FYarnProjectSynchronizer::AbsoluteSourcePath(const UYarnProjectAsset* YarnProjectAsset, const FString& SourcePath)
{
	FString S = FPaths::IsRelative(SourcePath) ? FPaths::Combine(YarnProjectAsset->YarnProjectPath(), SourcePath) : SourcePath;
	FPaths::NormalizeDirectoryName(S);
	FPaths::CollapseRelativeDirectories(S);
	FPaths::RemoveDuplicateSlashes(S);
	return S;
}


void FYarnProjectSynchronizer::UpdateLocAssets(const UYarnProjectAsset* YarnProjectAsset, const FString& Loc, const FString& LocAssets) const
{
	// UpdateYarnProjectAssetLocalizationAssets(YarnProjectAsset, Loc, LocAssets);
	const FString LocAssetsPath = AbsoluteSourcePath(YarnProjectAsset, LocAssets);
			
	TArray<FString> LocAssetSourceFiles;
	IFileManager::Get().FindFiles(LocAssetSourceFiles, *LocAssetsPath, TEXT(".wav")); 
	
	UpdateYarnProjectAssets<USoundWave>(
		YarnProjectAsset,
		LocAssetsPath,
		Loc,
		LocAssetSourceFiles,
		[](const FString& SourceFile, const FString& DestinationPackage) {
			return FAssetToolsModule::GetModule().Get().ImportAssets({SourceFile}, DestinationPackage);
		},
		[](USoundWave* Asset) {
			return FReimportManager::Instance()->Reimport(Asset, false, true, "", nullptr, INDEX_NONE, false, true);
		}
	);
	
	// TODO: what other asset file types do we support?
}


void FYarnProjectSynchronizer::UpdateLocStrings(const UYarnProjectAsset* YarnProjectAsset, const FString& Loc, const FString& LocStrings) const
{
	// UpdateYarnProjectAssetLocalizationStrings(YarnProjectAsset, Loc, LocStrings);
	const FString LocStringsPath = FPaths::GetPath(AbsoluteSourcePath(YarnProjectAsset, LocStrings));

	// TODO: switch to StringTable
	UpdateYarnProjectAssets<UDataTable>(
		YarnProjectAsset,
		LocStringsPath,
		Loc,
		{LocStrings},
		[&LocFileImporter = LocFileImporter](const FString& SourceFile, const FString& DestinationPackage) {
			// TODO: create a new StringTable asset

			// TODO: read in the CSV file

			// TODO: add the strings to the StringTable
					
			YS_LOG("Importing localisation asset %s", *SourceFile)
			UAutomatedAssetImportData* ImportData = NewObject<UAutomatedAssetImportData>();
			ImportData->Filenames.Add(SourceFile);
			ImportData->DestinationPath = DestinationPackage;
			ImportData->bReplaceExisting = true;
			ImportData->Factory = LocFileImporter;
			return FAssetToolsModule::GetModule().Get().ImportAssetsAutomated(ImportData);
		},
		[](UDataTable* Asset) {
			// TODO: manual reimport -- clear the string table contents, read the csv file, fill the string table
					
			return FReimportManager::Instance()->Reimport(Asset, false, true, "", nullptr, INDEX_NONE, false, true);
		}
	);
}


void FYarnProjectSynchronizer::UpdateYarnProjectAssetLocalizations(const UYarnProjectAsset* YarnProjectAsset) const
{
	// Check if other project assets need to be imported/updated/removed
	TOptional<FYarnProjectMetaData> ProjectMeta = FYarnProjectMetaData::FromAsset(YarnProjectAsset);
	if (ProjectMeta.IsSet())
	{
		YS_LOG(".yarnproject file parsed successfully; updating out of date assets for %s...", *YarnProjectAsset->GetName())

		for (auto& Localisation : ProjectMeta->localisation)
		{
			const FString& Loc = Localisation.Key;
			const FString& LocAssets = Localisation.Value.assets;
			const FString& LocStrings = Localisation.Value.strings;
			
			UpdateLocAssets(YarnProjectAsset, Loc, LocAssets);
			UpdateLocStrings(YarnProjectAsset, Loc, LocStrings);
		}
	}
}


template <class AssetClass>
void FYarnProjectSynchronizer::UpdateYarnProjectAssets(const UYarnProjectAsset* YarnProjectAsset, const FString& SourcesPath, const FString& Loc, const TArray<FString>& LocSources, TFunction<TArray<UObject*>(const FString& SourceFile, const FString& DestinationPackage)> ImportNew, TFunction<bool(AssetClass* Asset)> Reimport, TSubclassOf<UObject> TheAssetClass) const
{
	YS_LOG_FUNCSIG

	// FString SourcesList;
	// for (auto S : LocSources) SourcesList += S + TEXT(", ");
	// SourcesList.RemoveFromEnd(TEXT(", "));
	// YS_LOG("Found %i %s sources in %s: %s", LocSources.Num(), *TheAssetClass->GetName(), *SourcesPath, *SourcesList)
	
	if (LocSources.Num() == 0)
		return;
	
	// Check for existing localised asset files
	YS_LOG("Localised files found for language '%s'.  Looking for changes...", *Loc)
	
	const FString AssetDir = YarnProjectAsset->GetName() + TEXT("_Loc");
	
	const FString LocalisedAssetPackage = FPaths::Combine(FPaths::GetPath(YarnProjectAsset->GetPathName()), AssetDir, Loc);
	
	const FString LocalisedAssetPath = FPackageName::LongPackageNameToFilename(LocalisedAssetPackage);
	if (!FPaths::DirectoryExists(LocalisedAssetPath))
	{
		// YS_LOG("Creating localised asset content directory %s", *LocalisedAssetPath)
		IFileManager::Get().MakeDirectory(*LocalisedAssetPath);
	}

	// Find existing assets of the correct type in the expected package so we can check if they need to be updated/removed 
	TArray<FAssetData> ExistingAssets;
	FARFilter LocAssetFilter;
	LocAssetFilter.PackagePaths.Add(FName(LocalisedAssetPackage));
	LocAssetFilter.ClassNames.Add(TheAssetClass->GetFName());
	FAssetRegistryModule::GetRegistry().GetAssets(LocAssetFilter, ExistingAssets);

	TMap<FString, FAssetData> ExistingAssetsMap;
	TMap<FString, bool> ExistingAssetSourceSeen;
	for (const FAssetData& ExistingAsset : ExistingAssets)
	{
		ExistingAssetsMap.Add(ExistingAsset.AssetName.ToString(), ExistingAsset);
		ExistingAssetSourceSeen.Add(ExistingAsset.AssetName.ToString(), false);
		YS_LOG("Found existing imported asset %s", *ExistingAsset.AssetName.ToString())
	}

	// Read the list of assets, check if there is a matching unreal asset in the expected location
	for (const FString& LocSourceFile : LocSources)
	{
		// const FString LocAssetSourceFile = FPaths::IsRelative(LocStrings) ? FPaths::Combine(YarnProjectAsset->YarnProjectPath(), LocStrings) : LocStrings;
		
		const FString BaseName = ObjectTools::SanitizeObjectName(FPaths::GetBaseFilename(LocSourceFile));
		const FString FullLocSourceFilePath = FPaths::IsRelative(LocSourceFile) ? FPaths::Combine(SourcesPath, LocSourceFile) : LocSourceFile;

		// YS_LOG("BaseName: %s, File: %s", *BaseName, *LocSourceFile)

		// Check the imported asset exists in the expected location
		// YS_LOG("Checking for existing imported asset for localised file %s", *LocAssetSourceFile)

		if (ExistingAssetsMap.Contains(BaseName))
		{
			// YS_LOG("Found existing imported asset %s", *BaseName)
			ExistingAssetSourceSeen[BaseName] = true;

			// Check if the asset is up to date
			// typedef AssetClass->GetClass() AssetType;
			FAssetData& ExistingAssetData = ExistingAssetsMap[BaseName];
			AssetClass* ExistingAsset = Cast<AssetClass>(ExistingAssetData.GetAsset());
			if (!ExistingAsset)
			{
				YS_WARN("Could not load asset data for %s", *LocSourceFile)
			}
			else
			{
				const auto SourceFile = ExistingAsset->AssetImportData->SourceData.SourceFiles[0];
				if (SourceFile.Timestamp != IFileManager::Get().GetTimeStamp(*FullLocSourceFilePath) && SourceFile.FileHash != FMD5Hash::HashFile(*FullLocSourceFilePath))
				{
					YS_LOG("Existing asset %s is out of date and will be reimported", *ExistingAsset->GetName())
					if (!Reimport(ExistingAsset))
					{
						YS_WARN("Reimport of asset %s failed", *ExistingAsset->GetName())
					}
				}
			}
		}
		else
		{
			YS_LOG("Importing source file %s", *LocSourceFile)
			if (ImportNew(FullLocSourceFilePath, LocalisedAssetPackage).Num() == 0)
			{
				YS_WARN("Import of source file %s failed", *LocSourceFile)
			}
		}
	}
	
	// Delete unseen/old assets
	for (auto& ExistingAsset : ExistingAssetSourceSeen)
	{
		if (!ExistingAsset.Value)
		{
			YS_LOG("Deleting unused localisation asset %s", *ExistingAsset.Key)
			ObjectTools::DeleteSingleObject(ExistingAssetsMap[ExistingAsset.Key].GetAsset());
			ObjectTools::DeleteAssets({ExistingAssetsMap[ExistingAsset.Key]}, false);
		}
	}
}




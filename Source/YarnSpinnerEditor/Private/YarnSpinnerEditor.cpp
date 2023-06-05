// Copyright Epic Games, Inc. All Rights Reserved.

#include "YarnSpinnerEditor.h"

#include "AssetToolsModule.h"
#include "FileHelpers.h"
#include "IAssetTools.h"

#include "IYarnSpinnerModuleInterface.h"
#include "YarnAssetActions.h"
#include "YarnAssetFactory.h"
#include "YarnProjectAsset.h"
#include "YarnProjectMeta.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorFramework/AssetImportData.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/YSLogging.h"
#include "Sound/SoundWave.h"


DEFINE_LOG_CATEGORY(LogYarnSpinnerEditor);


void FYarnSpinnerEditor::AddModuleListeners()
{
	// add tools later
}


void FYarnSpinnerEditor::OnAssetAdded(const FAssetData& AssetData)
{
	YS_LOG_FUNCSIG
}


void FYarnSpinnerEditor::OnAssetRemoved(const FAssetData& AssetData)
{
	YS_LOG_FUNCSIG
}


void FYarnSpinnerEditor::OnAssetRenamed(const FAssetData& AssetData, const FString& OldObjectPath)
{
	YS_LOG_FUNCSIG
}


void FYarnSpinnerEditor::OnAssetRegistryFilesLoaded() const
{
	YS_LOG_FUNCSIG

	{
		// FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		// IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
		//
		// FARFilter Filter;
		// Filter.ClassNames.Add(UYarnProjectAsset::StaticClass()->GetFName());
		//
		// TArray<FAssetData> Assets;
		// AssetRegistry.GetAssets(Filter, Assets);
		//
		// YarnProjectAssets = Assets;

		UpdateAssetsAsNecessary();
	}
}


void FYarnSpinnerEditor::UpdateAssetsAsNecessary() const
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FARFilter YarnProjectFilter;
	YarnProjectFilter.ClassNames.Add(UYarnProjectAsset::StaticClass()->GetFName());

	TArray<FAssetData> YarnProjectAssets;
	AssetRegistry.GetAssets(YarnProjectFilter, YarnProjectAssets);

	for (auto Asset : YarnProjectAssets)
	{
		UYarnProjectAsset* YarnProjectAsset = Cast<UYarnProjectAsset>(Asset.GetAsset());
		YS_LOG("YarnProject Asset found: %s", *Asset.AssetName.ToString());

		// Check if yarn sources need a recompile
		TArray<FString> Sources;
		UYarnAssetFactory::GetSourcesForProject(YarnProjectAsset, Sources);
		if (YarnProjectAsset->ShouldRecompile(Sources))
		{
			YS_LOG(".yarnproject file is out of date; recompiling %s...", *Asset.AssetName.ToString())
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

		// Check if other project assets need to be imported/updated/removed
		TOptional<FYarnProjectMetaData> ProjectMeta = FYarnProjectMetaData::FromAsset(YarnProjectAsset);
		if (ProjectMeta.IsSet())
		{
			YS_LOG(".yarnproject file parsed successfully; updating out of date assets for %s...", *Asset.AssetName.ToString())

			for (auto& Localisation : ProjectMeta->localisation)
			{
				const FString& Loc = Localisation.Key;
				const FString& LocAssets = Localisation.Value.assets;
				const FString& LocStrings = Localisation.Value.strings;
				if (LocAssets.Len() > 0)
				{
					// check for localised asset files
					YS_LOG("Localised audio files found for language %s", *Localisation.Key)
					const FString AssetDir = YarnProjectAsset->GetName() + TEXT("_Loc");
					const FString LocalisedAssetPackage = FPaths::Combine(FPaths::GetPath(YarnProjectAsset->GetPathName()), AssetDir, Loc);
					const FString LocalisedAssetPath = FPackageName::LongPackageNameToFilename(LocalisedAssetPackage);
					if (!FPaths::DirectoryExists(LocalisedAssetPath))
					{
						// YS_WARN("Missing localised asset directory %s", *LocalisedAssetPath)
						IFileManager::Get().MakeDirectory(*LocalisedAssetPath);
					}

					// read the list of assets, check if there is a matching unreal asset in the expected location
					const FString LocAssetSourcePath = FPaths::IsRelative(LocAssets) ? FPaths::Combine(YarnProjectAsset->YarnProjectPath(), LocAssets) : LocAssets;
					TArray<FString> LocAssetSourceFiles;
					IFileManager::Get().FindFiles(LocAssetSourceFiles, *LocAssetSourcePath, TEXT(".wav")); // TODO: what other asset files do we support?

					FString RelativeSourcePath = LocAssetSourcePath;
					FPaths::MakePathRelativeTo(RelativeSourcePath, *FPaths::ProjectContentDir());

					TArray<FAssetData> ExistingAssets;
					FARFilter LocAssetFilter;
					LocAssetFilter.PackagePaths.Add(FName(LocalisedAssetPackage));
					AssetRegistry.GetAssets(LocAssetFilter, ExistingAssets);
					// YS_WARN("found %d imported assets in %s", ExistingAssets.Num(), *LocalisedAssetPackage)
					// YS_WARN("maybe should try this %s", *Asset.ObjectPath.ToString())

					TMap<FString, FAssetData> ExistingAssetsMap;
					TMap<FString, bool> ExistingAssetSourceSeen;
					for (const FAssetData& ExistingAsset : ExistingAssets)
					{
						ExistingAssetsMap.Add(ExistingAsset.AssetName.ToString(), ExistingAsset);
						ExistingAssetSourceSeen.Add(ExistingAsset.AssetName.ToString(), false);
					}

					for (const FString& LocAssetSourceFile : LocAssetSourceFiles)
					{
						const FString BaseName = FPaths::GetBaseFilename(LocAssetSourceFile);
						const FString FullLocAssetSourceFilePath = FPaths::Combine(LocAssetSourcePath, LocAssetSourceFile);

						// check the imported asset exists in the expected location
						YS_LOG("Checking for existing imported asset for localised file %s", *LocAssetSourceFile)

						if (ExistingAssetsMap.Contains(BaseName))
						{
							YS_LOG("found existing imported asset %s", *BaseName)
							ExistingAssetSourceSeen[BaseName] = true;
							// check if the asset is up to date
							FAssetData& ExistingAssetData = ExistingAssetsMap[BaseName];
							USoundWave* ExistingAsset = Cast<USoundWave>(ExistingAssetData.GetAsset());
							if (!ExistingAsset)
							{
								YS_WARN("Could not load asset data for %s", *LocAssetSourceFile)
							}
							else
							{
								const auto SourceFile = ExistingAsset->AssetImportData->SourceData.SourceFiles[0];
								if (SourceFile.Timestamp != IFileManager::Get().GetTimeStamp(*FullLocAssetSourceFilePath) && SourceFile.FileHash != FMD5Hash::HashFile(*FullLocAssetSourceFilePath))
								{
									YS_LOG("Existing asset %s is out of date and will be reimported", *LocAssetSourceFile)
									if (FReimportManager::Instance()->Reimport(ExistingAsset, false, true, "", nullptr, INDEX_NONE, false, true))
									{
										YS_LOG("Reimport succeeded");
									}
									else
									{
										YS_WARN("Reimport failed");
									}
								}
							}
						}
						else
						{
							AssetToolsModule.Get().ImportAssets({ FullLocAssetSourceFilePath }, LocalisedAssetPackage);
						}

						// TArray<FString> ToImport;
						// if (!ImportedAssets.ContainsByPredicate([&](const FAssetData& A)
						// {
						// 	YS_LOG("comparing %s and %s", *A.AssetName.ToString(), *BaseName)
						// 	return A.GetAsset()->GetName() == BaseName;
						// }))
						// {
						// 	ToImport.Add(FPaths::Combine(LocAssetSourcePath, LocAssetSourceFile));
						// }
						// if (!ToImport.Num() == 0)
						// {
						// 	YS_LOG("importing %i new localised assets", ToImport.Num())
						// 	AssetToolsModule.Get().ImportAssets(ToImport, LocalisedAssetPackage);
						// }
					}

					// TODO: delete unseen/old assets
				}
			}


			// TODO: update out of date assets
			// 1. build a list of assets referenced by the project file
			// TArray<FString> Files = ProjectMeta->GetYarnSourceFiles();

			// 2. compare with assets already imported


			// ---- how?? where is the resource info? need to store it in the asset


			// 3. import/reimport as necessary


			// TODO: monitor yarn asset folders for changes (esp. new & deleted files)
		}
	}
}


void FYarnSpinnerEditor::StartupModule()
{
	YS_LOG_FUNCSIG

	// Register custom types:
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		// add custom category
		EAssetTypeCategories::Type YarnSpinnerAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("YarnSpinner")), FText::FromString("YarnSpinner"));

		// register our custom asset with example category
		TSharedPtr<IAssetTypeActions> Action = MakeShareable(new FYarnAssetActions(YarnSpinnerAssetCategory));
		AssetTools.RegisterAssetTypeActions(Action.ToSharedRef());

		// saved it here for unregister later
		CreatedAssetTypeActions.Add(Action);
	}

	// Set up asset registry delegates
	{
		const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		OnAssetRegistryFilesLoadedHandle = AssetRegistry.OnFilesLoaded().AddRaw(this, &FYarnSpinnerEditor::OnAssetRegistryFilesLoaded);
		// TODO: update on asset change
	}

	IYarnSpinnerModuleInterface::StartupModule();

	UpdateAssetsAsNecessary();
}


void FYarnSpinnerEditor::ShutdownModule()
{
	// Unregister all the asset types that we registered
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (int32 i = 0; i < CreatedAssetTypeActions.Num(); ++i)
		{
			AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[i].ToSharedRef());
		}
	}
	CreatedAssetTypeActions.Empty();

	if (FAssetRegistryModule* AssetRegistryModule = FModuleManager::GetModulePtr<FAssetRegistryModule>("AssetRegistry"))
	{
		AssetRegistryModule->Get().OnFilesLoaded().Remove(OnAssetRegistryFilesLoadedHandle);
	}

	IYarnSpinnerModuleInterface::ShutdownModule();
}


IMPLEMENT_MODULE(FYarnSpinnerEditor, YarnSpinnerEditor)

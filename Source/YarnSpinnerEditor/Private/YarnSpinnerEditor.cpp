// Copyright Epic Games, Inc. All Rights Reserved.

#include "YarnSpinnerEditor.h"

#include "AssetToolsModule.h"
#include "IAssetTools.h"

#include "IYarnSpinnerModuleInterface.h"
#include "YarnAssetActions.h"
#include "YarnAssetFactory.h"
#include "YarnProjectAsset.h"
#include "YarnProjectMeta.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/YSLogging.h"


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


void FYarnSpinnerEditor::OnAssetRegistryFilesLoaded()
{
	YS_LOG_FUNCSIG

	{
    	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		// FPaths::GetRelativePathToRoot();
	
    	// TArray<FString> ContentPaths;
    	// ContentPaths.Add("/Game");
    	// ContentPaths.Add("/Game/Plugins/YarnSpinner-Unreal/Content");
    	// AssetRegistry.ScanPathsSynchronous(ContentPaths);

    	FARFilter Filter;
    	Filter.ClassNames.Add(UYarnProjectAsset::StaticClass()->GetFName());

		TArray<FAssetData> Assets;
		AssetRegistry.GetAssets(Filter, Assets);

		YarnProjectAssets = Assets;

		UpdateAssetsAsNecessary();
	}
}


void FYarnSpinnerEditor::UpdateAssetsAsNecessary()
{
	for (auto Asset : YarnProjectAssets)
	{
		UYarnProjectAsset* YarnProjectAsset = Cast<UYarnProjectAsset>(Asset.GetAsset());
		YS_LOG("YarnProject Asset found: %s", *Asset.AssetName.ToString());

		// Check if yarn sources need a recompile
		UYarnAssetFactory::YscPath();

		TArray<FString> Sources;
		UYarnAssetFactory::GetSourcesForProject(YarnProjectAsset, Sources);
		if (YarnProjectAsset->ShouldRecompile(Sources))
		{
			YS_LOG(".yarnproject file is out of date; recompiling %s...", *Asset.AssetName.ToString())
			// YarnProjectAsset->Recompile();
			// TODO: recompile
		}

		// Check if other project assets need to be imported/updated/removed
		TOptional<FYarnProjectMetaData> ProjectMeta = FYarnProjectMetaData::FromAsset(YarnProjectAsset);
		if (ProjectMeta.IsSet())
		{
			YS_LOG(".yarnproject file parsed successfully; updating out of date assets for %s...", *Asset.AssetName.ToString())
			
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
    	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
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

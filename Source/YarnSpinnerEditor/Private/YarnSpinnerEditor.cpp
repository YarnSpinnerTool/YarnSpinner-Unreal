// Copyright Epic Games, Inc. All Rights Reserved.

#include "YarnSpinnerEditor.h"

#include "YarnSpinner.h"

#include "IYarnSpinnerModuleInterface.h"

//IMPLEMENT_GAME_MODULE(FToolExampleEditor, ToolExampleEditor)

void FYarnSpinnerEditor::AddModuleListeners()
{
    // add tools later
}

void FYarnSpinnerEditor::StartupModule()
{
    // register custom types:
    {
        IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
        
        // add custom category
        EAssetTypeCategories::Type ExampleCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Yarn")), FText::FromString("Yarn"));
        
        // register our custom asset with example category
        TSharedPtr<IAssetTypeActions> Action = MakeShareable(new FYarnAssetActions(ExampleCategory));
        AssetTools.RegisterAssetTypeActions(Action.ToSharedRef());
        
        // saved it here for unregister later
        CreatedAssetTypeActions.Add(Action);
    }
    
    IYarnSpinnerModuleInterface::StartupModule();
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
    
    IYarnSpinnerModuleInterface::ShutdownModule();
}

IMPLEMENT_MODULE(FYarnSpinnerEditor, YarnSpinnerEditor)

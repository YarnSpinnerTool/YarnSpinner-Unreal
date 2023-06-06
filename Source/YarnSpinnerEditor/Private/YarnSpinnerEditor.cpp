// Copyright Epic Games, Inc. All Rights Reserved.

#include "YarnSpinnerEditor.h"

#include "AssetToolsModule.h"
#include "IAssetTools.h"

#include "IYarnSpinnerModuleInterface.h"
#include "YarnAssetActions.h"
#include "YarnAssetFactory.h"
#include "YarnProjectSynchronizer.h"
#include "Misc/YSLogging.h"


DEFINE_LOG_CATEGORY(LogYarnSpinnerEditor);


void FYarnSpinnerEditor::AddModuleListeners()
{
	// add tools later
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

	YarnProjectSynchronizer = MakeUnique<FYarnProjectSynchronizer>();

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

	YarnProjectSynchronizer.Reset();

	IYarnSpinnerModuleInterface::ShutdownModule();
}


IMPLEMENT_MODULE(FYarnSpinnerEditor, YarnSpinnerEditor)

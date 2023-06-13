// Copyright Epic Games, Inc. All Rights Reserved.

#include "YarnSpinnerEditor.h"

#include "AssetToolsModule.h"
#include "DisplayLine.h"
#include "IAssetTools.h"

#include "IYarnSpinnerModuleInterface.h"
#include "YarnAssetActions.h"
#include "YarnAssetFactory.h"
#include "YarnProjectSynchronizer.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Factories/CSVImportFactory.h"
#include "Factories/DataTableFactory.h"
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

	LocFileImporter = NewObject<UCSVImportFactory>();
	LocFileImporter->AddToRoot();
	UDataTable* ImportOptions = NewObject<UDataTable>();
	ImportOptions->RowStruct = FDisplayLine::StaticStruct();
	ImportOptions->RowStructName = "DisplayLine";
	ImportOptions->bIgnoreExtraFields = true;
	ImportOptions->bIgnoreMissingFields = false;
	ImportOptions->ImportKeyField = "id"; // becomes the Name field of the DataTable
	LocFileImporter->DataTableImportOptions = ImportOptions;
	// The FCSVImportSettings struct is not part of the dll export so we have to use the JSON API to set it up
	TSharedRef<FJsonObject> CSVImportSettings = MakeShareable(new FJsonObject());
	CSVImportSettings->SetField("ImportType", MakeShareable(new FJsonValueNumber(0)));
	CSVImportSettings->SetField("ImportRowStruct", MakeShareable(new FJsonValueString("DisplayLine")));
	LocFileImporter->ParseFromJson(CSVImportSettings);

	YarnProjectSynchronizer = MakeUnique<FYarnProjectSynchronizer>();
	YarnProjectSynchronizer->SetLocFileImporter(LocFileImporter);

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

	LocFileImporter->RemoveFromRoot();
	LocFileImporter = nullptr;

	IYarnSpinnerModuleInterface::ShutdownModule();
}


IMPLEMENT_MODULE(FYarnSpinnerEditor, YarnSpinnerEditor)

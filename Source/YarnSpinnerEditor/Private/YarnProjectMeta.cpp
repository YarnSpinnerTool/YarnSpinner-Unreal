#include "YarnProjectMeta.h"

#include "JsonObjectConverter.h"
#include "EditorFramework/AssetImportData.h"
#include "Misc/YSLogging.h"
#include "Misc/FileHelper.h"


TOptional<FYarnProjectMetaData> FYarnProjectMetaData::FromAsset(const UYarnProjectAsset* Asset)
{
	TOptional<FYarnProjectMetaData> MetaData;

	if (!Asset)
	{
		YS_ERR("No asset provided to YarnProjectMeta constructor")
		return MetaData;
	}
	
	if (!Asset->AssetImportData)
	{
		YS_ERR("Yarn Project asset is missing import data")
		return MetaData;
	}

	bool bYarnProjectFileFound = false;

	auto SourceFiles = Asset->AssetImportData->ExtractFilenames();

	for (auto& File : SourceFiles)
	{
		YS_LOG("Found source file %s", *File);

		if (File.EndsWith(".yarnproject"))
		{
			bYarnProjectFileFound = true;
		
			// Read the file into a string
			FString FileContents;

			if (!FFileHelper::LoadFileToString(FileContents, *File))
			{
				YS_ERR("Could not load .yarnproject file %s", *File)
				return MetaData;
			}
			
			// YS_LOG(".yarproject file contents:\n%s", *FileContents)

			// Parse the JSON
			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(FileContents);
			if (!FJsonSerializer::Deserialize(JsonReader, JsonObject) || !JsonObject.IsValid())
			{
				YS_ERR("Could not parse .yarnproject file %s -- invalid format", *File)
				return MetaData;
			}

			// TODO: check .yarnproject format version before attempting to parse into a struct ?
			// i.e.: if (JsonObject->HasField("projectFileVersion") && JsonObject->GetIntegerField("projectFileVersion") != 2) { ... }
			
			FYarnProjectMetaData Meta;
			if (!FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &Meta, 0, 0))
			// if (!FJsonObjectConverter::JsonObjectStringToUStruct(FileContents, &Meta, 0, 0)) // if not testing the yarn project file version above then this one line is all that's needed after '// Parse the JSON'
			{
				YS_ERR("Could not parse .yarnproject file %s", *File)
				return MetaData;
			}

			MetaData = Meta;
		}
	}

	if (!bYarnProjectFileFound)
	{
		YS_ERR("Could not find .yarporject file for project %s", *Asset->GetName())
	}

	return MetaData;
}

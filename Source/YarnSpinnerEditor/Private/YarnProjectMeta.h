#pragma once
#include "YarnProject.h"
#include "CoreMinimal.h"
#include "YarnProjectMeta.generated.h"


USTRUCT()
struct YARNSPINNEREDITOR_API FYarnProjectLocalizationData
{
	GENERATED_BODY()

	UPROPERTY()
	FString assets;

	UPROPERTY()
	FString strings;
};


USTRUCT()
struct YARNSPINNEREDITOR_API FYarnProjectMetaData
{
	GENERATED_BODY()

	static TOptional<FYarnProjectMetaData> FromAsset(const UYarnProject* Asset);
	
	UPROPERTY()
	int32 projectFileVersion = -1;

	UPROPERTY()
	TArray<FString> sourceFiles;
	
	UPROPERTY()
	TArray<FString> excludeFiles;

	UPROPERTY()
	TMap<FString, FYarnProjectLocalizationData> localisation;

	UPROPERTY()
	FString baseLanguage = "en";
	
	UPROPERTY()
	FString definitions;

	// TODO: confirm this is the correct format -- all the examples contain an empty object {}
	UPROPERTY()
	TMap<FString, FString> compilerOptions;

	// The path to the .yarnproject file
	FString YarnProjectFilePath;
};


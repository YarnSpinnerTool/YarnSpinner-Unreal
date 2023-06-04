// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "YarnProjectAsset.generated.h"


USTRUCT()
struct FYarnSourceMeta
{
	GENERATED_BODY()

	// Timestamp of the file when it was imported.
	UPROPERTY(VisibleAnywhere)
	FDateTime Timestamp;

	// MD5 hash of the file when it was imported.
	UPROPERTY()
	FString FileHash;

	FString ToString() const
	{
		return FString::Printf(TEXT("%s -- %s"), *Timestamp.ToString(), *FileHash);
	}
};


/**
 * 
 */
UCLASS(Blueprintable, AutoExpandCategories = "ImportOptions")
class YARNSPINNER_API UYarnProjectAsset : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<uint8> Data;

	UPROPERTY(VisibleAnywhere, Category="Yarn Spinner")
	TMap<FName, FString> Lines;

	UPROPERTY(VisibleAnywhere, Category="File Path")
	TMap<FString, FYarnSourceMeta> YarnFiles;

#if WITH_EDITORONLY_DATA
	virtual void PostInitProperties() override;
	void SetYarnSources(const TArray<FString>& NewYarnSources);
	bool ShouldRecompile(const TArray<FString>& LatestYarnSources) const;
	FString YarnProjectPath() const;

	/** The file this data table was imported from, may be empty */
	UPROPERTY(VisibleAnywhere, Instanced, Category=ImportSource)
	class UAssetImportData* AssetImportData;
#endif
};

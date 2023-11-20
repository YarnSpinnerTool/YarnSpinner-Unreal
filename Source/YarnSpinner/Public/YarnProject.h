// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "YarnProject.generated.h"


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
UCLASS(AutoExpandCategories = "ImportOptions")
class YARNSPINNER_API UYarnProject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<uint8> Data;

	UPROPERTY(VisibleAnywhere, Category="Yarn Spinner")
	TMap<FName, FString> Lines;

	// Yarn files that were imported into this project, relative to the .yarnproject file, mapped to file metadata.
	UPROPERTY(VisibleAnywhere, Category="File Path")
	TMap<FString, FYarnSourceMeta> YarnFiles;

    // UPROPERTY(EditDefaultsOnly, Category = "Yarn Spinner")
    // TArray<TSubclassOf<class AYarnFunctionLibrary>> FunctionLibraries;

    void Init();

    FString GetLocAssetPackage() const;
    FString GetLocAssetPackage(FName Language) const;
    class UDataTable* GetLocTextDataTable(FName Language) const;

    TArray<TSoftObjectPtr<UObject>> GetLineAssets(FName Name);

#if WITH_EDITORONLY_DATA
	virtual void PostInitProperties() override;
	void SetYarnSources(const TArray<FString>& NewYarnSources);
	bool ShouldRecompile(const TArray<FString>& LatestYarnSources) const;
	FString YarnProjectPath() const;

    /** The file this data table was imported from, may be empty */
	UPROPERTY(VisibleAnywhere, Instanced, Category=ImportSource)
	class UAssetImportData* AssetImportData;
#endif

private:
    TMap<FName, TArray<TSoftObjectPtr<UObject>>> LineAssets;
};

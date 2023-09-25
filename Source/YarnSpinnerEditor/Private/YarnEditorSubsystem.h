// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "Library/YarnFunctionLibrary.h"
#include "YarnEditorSubsystem.generated.h"

USTRUCT()
struct FYarnCallableBlueprintArgument
{
    GENERATED_BODY()
    
public:
    FString Name;
    FString Type;
    FString DefaultValue;
};

USTRUCT()
struct FYarnCallableBlueprintFunction
{
    GENERATED_BODY()

public:
    FString Name;
    FString Parent;
    TArray<FYarnCallableBlueprintArgument> Arguments;
    FString ReturnType = "void";

    FString ToString()
    {
        TArray<FString> Args;
        for (auto Arg : Arguments)
        {
            Args.Add(FString::Printf(TEXT("%s %s = %s"), *Arg.Type, *Arg.Name, *Arg.DefaultValue));
        }
        return FString::Printf(TEXT("%s %s(%s)"), *ReturnType, *Name, *FString::Join(Args, TEXT(", ")));
    }
};

/**
 * 
 */
UCLASS()
class YARNSPINNEREDITOR_API UYarnEditorSubsystem : public UEditorSubsystem
{
    GENERATED_BODY()

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

private:
    UPROPERTY()
    UYarnFunctionLibrary* YarnFunctionRegistry;

    class FFindInBlueprintSearchManager* BlueprintSearchManager;
	FDelegateHandle OnAssetRegistryFilesLoadedHandle;
	FDelegateHandle OnAssetAddedHandle;
	FDelegateHandle OnAssetRemovedHandle;
	FDelegateHandle OnAssetRenamedHandle;
    
    // Callback for when the asset registry has finished scanning assets on Unreal Editor load.
	void OnAssetRegistryFilesLoaded();
	void OnAssetAdded(const FAssetData& AssetData) const;
	void OnAssetRemoved(const FAssetData& AssetData) const;
	void OnAssetRenamed(const FAssetData& AssetData, const FString& OldObjectPath) const;
};

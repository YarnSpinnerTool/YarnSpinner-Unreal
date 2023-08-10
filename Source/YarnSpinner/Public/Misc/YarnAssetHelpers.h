#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"


class FYarnAssetHelpers
{
public:
    template <class AssetClass>
    static TArray<FAssetData> FindAssetsInRegistry(const TSubclassOf<UObject> BaseClass = AssetClass::StaticClass());
    template <class AssetClass>
    static TArray<FAssetData> FindAssetsInRegistryByPackageName(const FName SearchPackage, const TSubclassOf<UObject> BaseClass = AssetClass::StaticClass());
    template <class AssetClass>
    static TArray<FAssetData> FindAssetsInRegistryByPackagePath(const FName SearchPackage, const TSubclassOf<UObject> BaseClass = AssetClass::StaticClass());
    template <class AssetClass>
    static TArray<FAssetData> FindAssetsInRegistryByPackagePath(const FString SearchPackage, const TSubclassOf<UObject> BaseClass = AssetClass::StaticClass());

private:
    template <class AssetClass>
    static FARFilter GetClassPathFilter(const TSubclassOf<UObject> BaseClass = AssetClass::StaticClass());
    static TArray<FAssetData> FindAssets(const FARFilter& AssetFilter);
};


template <class AssetClass>
TArray<FAssetData> FYarnAssetHelpers::FindAssetsInRegistry(const TSubclassOf<UObject> BaseClass)
{
   return FindAssets(GetClassPathFilter<AssetClass>(BaseClass));
}


template <class AssetClass>
TArray<FAssetData> FYarnAssetHelpers::FindAssetsInRegistryByPackageName(const FName SearchPackage, const TSubclassOf<UObject> BaseClass)
{
    FARFilter AssetFilter = GetClassPathFilter<AssetClass>(BaseClass);
    AssetFilter.PackageNames.Add(SearchPackage);
    return FindAssets(AssetFilter);
}


template <class AssetClass>
TArray<FAssetData> FYarnAssetHelpers::FindAssetsInRegistryByPackagePath(const FName SearchPackage, const TSubclassOf<UObject> BaseClass)
{
    FARFilter AssetFilter = GetClassPathFilter<AssetClass>(BaseClass);
    AssetFilter.PackagePaths.Add(SearchPackage);
    return FindAssets(AssetFilter);
}


template <class AssetClass>
TArray<FAssetData> FYarnAssetHelpers::FindAssetsInRegistryByPackagePath(const FString SearchPackage, const TSubclassOf<UObject> BaseClass)
{
    return FindAssetsInRegistryByPackagePath<AssetClass>(FName(SearchPackage), BaseClass);
}


template <class AssetClass>
FARFilter FYarnAssetHelpers::GetClassPathFilter(const TSubclassOf<UObject> BaseClass)
{
    FARFilter AssetFilter; 
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
    AssetFilter.ClassPaths.Add(BaseClass->GetClassPathName());
#else
    AssetFilter.ClassNames.Add(BaseClass->GetFName());
#endif
    return AssetFilter;
}


inline TArray<FAssetData> FYarnAssetHelpers::FindAssets(const FARFilter& AssetFilter)
{
    TArray<FAssetData> ExistingAssets;
    FAssetRegistryModule::GetRegistry().GetAssets(AssetFilter, ExistingAssets);
    return ExistingAssets;
}



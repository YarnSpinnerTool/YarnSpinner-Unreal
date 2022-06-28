#include "YarnAssetActions.h"
#include "YarnAsset.h"
#include "EditorFramework/AssetImportData.h"

FYarnAssetActions::FYarnAssetActions(EAssetTypeCategories::Type InAssetCategory)
    : MyAssetCategory(InAssetCategory)
{
}

FText FYarnAssetActions::GetName() const
{
    return FText::FromString("Yarn");
}

FColor FYarnAssetActions::GetTypeColor() const
{
    return FColor(230, 205, 165);
}

UClass* FYarnAssetActions::GetSupportedClass() const
{
    return UYarnAsset::StaticClass();
}

uint32 FYarnAssetActions::GetCategories()
{
    return MyAssetCategory;
}

void FYarnAssetActions::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (auto& Asset : TypeAssets)
	{
		const auto YarnAsset = CastChecked<UYarnAsset>(Asset);
		YarnAsset->AssetImportData->ExtractFilenames(OutSourceFilePaths);
	}
}

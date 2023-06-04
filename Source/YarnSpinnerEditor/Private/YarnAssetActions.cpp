#include "YarnAssetActions.h"
#include "YarnProjectAsset.h"
#include "EditorFramework/AssetImportData.h"

FYarnAssetActions::FYarnAssetActions(EAssetTypeCategories::Type InAssetCategory)
    : MyAssetCategory(InAssetCategory)
{
}

FText FYarnAssetActions::GetName() const
{
    return FText::FromString("Yarn Project");
}

FColor FYarnAssetActions::GetTypeColor() const
{
    return FColor(230, 205, 165);
}

UClass* FYarnAssetActions::GetSupportedClass() const
{
    return UYarnProjectAsset::StaticClass();
}

uint32 FYarnAssetActions::GetCategories()
{
    return MyAssetCategory;
}

void FYarnAssetActions::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (auto& Asset : TypeAssets)
	{
		const auto YarnAsset = CastChecked<UYarnProjectAsset>(Asset);
        if (YarnAsset && YarnAsset->AssetImportData) {
            YarnAsset->AssetImportData->ExtractFilenames(OutSourceFilePaths);
        }
	}
}

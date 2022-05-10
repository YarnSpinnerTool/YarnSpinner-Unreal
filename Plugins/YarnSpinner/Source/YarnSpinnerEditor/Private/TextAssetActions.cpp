#include "TextAssetActions.h"
#include "TextAsset.h"
#include "EditorFramework/AssetImportData.h"

FTextAssetActions::FTextAssetActions(EAssetTypeCategories::Type InAssetCategory)
    : MyAssetCategory(InAssetCategory)
{
}

FText FTextAssetActions::GetName() const
{
    return FText::FromString("Yarn");
}

FColor FTextAssetActions::GetTypeColor() const
{
    return FColor(230, 205, 165);
}

UClass* FTextAssetActions::GetSupportedClass() const
{
    return UTextAsset::StaticClass();
}

uint32 FTextAssetActions::GetCategories()
{
    return MyAssetCategory;
}

void FTextAssetActions::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (auto& Asset : TypeAssets)
	{
		const auto TextAsset = CastChecked<UTextAsset>(Asset);
		TextAsset->AssetImportData->ExtractFilenames(OutSourceFilePaths);
	}
}

#include "YarnAssetActions.h"
#include "YarnProject.h"
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
    return UYarnProject::StaticClass();
}


uint32 FYarnAssetActions::GetCategories()
{
    return MyAssetCategory;
}


void FYarnAssetActions::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
    for (auto& Asset : TypeAssets)
    {
        const auto YarnProject = CastChecked<UYarnProject>(Asset);
        if (YarnProject && YarnProject->AssetImportData)
        {
            YarnProject->AssetImportData->ExtractFilenames(OutSourceFilePaths);
        }
    }
}

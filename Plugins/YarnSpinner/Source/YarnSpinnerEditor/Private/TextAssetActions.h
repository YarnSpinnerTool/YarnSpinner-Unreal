#pragma once
#include "AssetTypeActions_Base.h"

class FTextAssetActions : public FAssetTypeActions_Base
{
public:
    FTextAssetActions(EAssetTypeCategories::Type InAssetCategory);

    // IAssetTypeActions interface
    virtual FText GetName() const override;
    virtual FColor GetTypeColor() const override;
    virtual UClass* GetSupportedClass() const override;
    virtual uint32 GetCategories() override;
    virtual bool IsImportedAsset() const override { return true; }
    virtual void GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const override;
    // End of IAssetTypeActions interface

private:
    EAssetTypeCategories::Type MyAssetCategory;
};

#include "ReimportYarnAssetFactory.h"

#include "CoreMinimal.h"
#include "EditorFramework/AssetImportData.h"
#include "Misc/YSLogging.h"


UReimportYarnAssetFactory::UReimportYarnAssetFactory(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}


bool UReimportYarnAssetFactory::FactoryCanImport(const FString& Filename)
{
    return true;
}


bool UReimportYarnAssetFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
    UYarnProject* YarnProject = Cast<UYarnProject>(Obj);
    if (YarnProject)
    {
        YarnProject->AssetImportData->ExtractFilenames(OutFilenames);

        // Always allow reimporting a yarn asset
        return true;
    }
    return false;
}


void UReimportYarnAssetFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
    UYarnProject* YarnProject = Cast<UYarnProject>(Obj);
    if (YarnProject && ensure(NewReimportPaths.Num() == 1))
    {
        YarnProject->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
    }
}


EReimportResult::Type UReimportYarnAssetFactory::Reimport(UObject* Obj)
{
    YS_LOG_FUNCSIG
    
    EReimportResult::Type Result = EReimportResult::Failed;
    if (UYarnProject* YarnProject = Cast<UYarnProject>(Obj))
    {
        // Result = EReimportResult::Failed;
        Result = UYarnAssetFactory::Reimport(YarnProject); // ? EReimportResult::Succeeded : EReimportResult::Failed;
        if (Result == EReimportResult::Succeeded)
        {
            Obj->MarkPackageDirty();
        }
    }
    return Result;
}


int32 UReimportYarnAssetFactory::GetPriority() const
{
    return ImportPriority;
}

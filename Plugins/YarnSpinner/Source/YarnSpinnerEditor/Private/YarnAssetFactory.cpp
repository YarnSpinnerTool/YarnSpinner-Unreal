// Fill out your copyright notice in the Description page of Project Settings.


#include "YarnAssetFactory.h"

#include "Misc/FileHelper.h"
#include "EditorFramework/AssetImportData.h"
#include "Containers/UnrealString.h"

#include "ReimportYarnAssetFactory.h"

UYarnAssetFactory::UYarnAssetFactory( const FObjectInitializer& ObjectInitializer )
    : Super(ObjectInitializer)
{
    Formats.Add(FString(TEXT("yarn;")) + NSLOCTEXT("UYarnAssetFactory", "FormatTxt", "Yarn File").ToString());
    Formats.Add(FString(TEXT("yarnproject;")) + NSLOCTEXT("UYarnAssetFactory", "FormatTxt", "Yarn Project").ToString());
    SupportedClass = UYarnAsset::StaticClass();
    bCreateNew = false;
    bEditorImport = true;
}

UObject* UYarnAssetFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
{
//    FEditorDelegates::OnAssetPreImport.Broadcast(this, InClass, InParent, InName, Type);
    
    UYarnAsset* TextAsset = nullptr;
    FString TextString;

    TextAsset = NewObject<UYarnAsset>(InParent, InClass, InName, Flags);
    // TextAsset->SourceFilePath = UAssetImportData::SanitizeImportFilename(CurrentFilename, TextAsset->GetOutermost());

    const TCHAR* fileName = *CurrentFilename;

    FString Result;
    if (FFileHelper::LoadFileToString(Result, fileName)) {
        TextAsset->Text = Result;
    } else {
        TextAsset->Text = FString("Failed to load!");
    }

    if (!CurrentFilename.IsEmpty())
    {
        TextAsset->AssetImportData->Update(CurrentFilename);
    }
    
//    FEditorDelegates::OnAssetPostImport.Broadcast(this, TextAsset);

    return TextAsset;
}

bool UYarnAssetFactory::FactoryCanImport(const FString& Filename) {
    return FPaths::GetExtension(Filename).Equals(TEXT("yarn"))
        || FPaths::GetExtension(Filename).Equals(TEXT("yarnproject"));
}

EReimportResult::Type UYarnAssetFactory::Reimport(UYarnAsset* TextAsset) {
    auto Path = TextAsset->AssetImportData->GetFirstFilename();

    
	if (Path.IsEmpty() == false)
	{ 
		FString FilePath = IFileManager::Get().ConvertToRelativePath(*Path);

		TArray<uint8> Data;
        
		if ( FFileHelper::LoadFileToArray(Data, *FilePath) )
		{
			const uint8* Ptr = Data.GetData();
			CurrentFilename = FilePath; //not thread safe but seems to be how it is done..
			bool bWasCancelled = false;
            UObject *Result = FactoryCreateBinary(TextAsset->GetClass(), TextAsset->GetOuter(), TextAsset->GetFName(), TextAsset->GetFlags(), nullptr, *FPaths::GetExtension(FilePath), Ptr, Ptr + Data.Num(), GWarn);

            // if (bWasCancelled)
			// {
			// 	return EReimportResult::Cancelled;
			// }
			return Result ? EReimportResult::Succeeded : EReimportResult::Failed;
		}
	}
	return EReimportResult::Failed;
}

///////////// Reimport

UReimportYarnAssetFactory::UReimportYarnAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

bool UReimportYarnAssetFactory::FactoryCanImport( const FString& Filename )
{
	return true;
}

bool UReimportYarnAssetFactory::CanReimport( UObject* Obj, TArray<FString>& OutFilenames )
{	
	UYarnAsset* DataTable = Cast<UYarnAsset>(Obj);
	if (DataTable)
	{
		DataTable->AssetImportData->ExtractFilenames(OutFilenames);
		
		// Always allow reimporting a text asset
		return true;
	}
	return false;
}

void UReimportYarnAssetFactory::SetReimportPaths( UObject* Obj, const TArray<FString>& NewReimportPaths )
{	
	UYarnAsset* DataTable = Cast<UYarnAsset>(Obj);
	if (DataTable && ensure(NewReimportPaths.Num() == 1))
	{
		DataTable->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type UReimportYarnAssetFactory::Reimport( UObject* Obj )
{	
	EReimportResult::Type Result = EReimportResult::Failed;
	if (UYarnAsset* DataTable = Cast<UYarnAsset>(Obj))
	{
        // Result = EReimportResult::Failed;
        Result = UYarnAssetFactory::Reimport(DataTable) ? EReimportResult::Succeeded : EReimportResult::Failed;
    }
	return Result;
}

int32 UReimportYarnAssetFactory::GetPriority() const
{
	return ImportPriority;
}

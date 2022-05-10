// Fill out your copyright notice in the Description page of Project Settings.


#include "Misc/FileHelper.h"
#include "EditorFramework/AssetImportData.h"
#include "Containers/UnrealString.h"

#include "TextAssetFactory.h"
#include "ReimportTextAssetFactory.h"

UTextAssetFactory::UTextAssetFactory( const FObjectInitializer& ObjectInitializer )
    : Super(ObjectInitializer)
{
    Formats.Add(FString(TEXT("yarn;")) + NSLOCTEXT("UTextAssetFactory", "FormatTxt", "Yarn File").ToString());
    Formats.Add(FString(TEXT("yarnproject;")) + NSLOCTEXT("UTextAssetFactory", "FormatTxt", "Yarn Project").ToString());
    SupportedClass = UTextAsset::StaticClass();
    bCreateNew = false;
    bEditorImport = true;
}

UObject* UTextAssetFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
{
//    FEditorDelegates::OnAssetPreImport.Broadcast(this, InClass, InParent, InName, Type);
    
    UTextAsset* TextAsset = nullptr;
    FString TextString;

    TextAsset = NewObject<UTextAsset>(InParent, InClass, InName, Flags);
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

bool UTextAssetFactory::FactoryCanImport(const FString& Filename) {
    return FPaths::GetExtension(Filename).Equals(TEXT("yarn"))
        || FPaths::GetExtension(Filename).Equals(TEXT("yarnproject"));
}

EReimportResult::Type UTextAssetFactory::Reimport(UTextAsset* TextAsset) {
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

UReimportTextAssetFactory::UReimportTextAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

bool UReimportTextAssetFactory::FactoryCanImport( const FString& Filename )
{
	return true;
}

bool UReimportTextAssetFactory::CanReimport( UObject* Obj, TArray<FString>& OutFilenames )
{	
	UTextAsset* DataTable = Cast<UTextAsset>(Obj);
	if (DataTable)
	{
		DataTable->AssetImportData->ExtractFilenames(OutFilenames);
		
		// Always allow reimporting a text asset
		return true;
	}
	return false;
}

void UReimportTextAssetFactory::SetReimportPaths( UObject* Obj, const TArray<FString>& NewReimportPaths )
{	
	UTextAsset* DataTable = Cast<UTextAsset>(Obj);
	if (DataTable && ensure(NewReimportPaths.Num() == 1))
	{
		DataTable->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type UReimportTextAssetFactory::Reimport( UObject* Obj )
{	
	EReimportResult::Type Result = EReimportResult::Failed;
	if (UTextAsset* DataTable = Cast<UTextAsset>(Obj))
	{
        // Result = EReimportResult::Failed;
        Result = UTextAssetFactory::Reimport(DataTable) ? EReimportResult::Succeeded : EReimportResult::Failed;
    }
	return Result;
}

int32 UReimportTextAssetFactory::GetPriority() const
{
	return ImportPriority;
}

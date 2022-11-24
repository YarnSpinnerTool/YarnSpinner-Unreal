// Fill out your copyright notice in the Description page of Project Settings.

#include "YarnAssetFactory.h"

#include "Misc/FileHelper.h"
#include "EditorFramework/AssetImportData.h"
#include "Containers/UnrealString.h"

#include "ReimportYarnAssetFactory.h"

#include "yarn_spinner.pb.h"
#include "compiler_output.pb.h"

#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/type_resolver_util.h>

google::protobuf::Message &from_json(google::protobuf::Message &msg,
								const std::string &json);

UYarnAssetFactory::UYarnAssetFactory( const FObjectInitializer& ObjectInitializer )
    : Super(ObjectInitializer)
{
    Formats.Add(FString(TEXT("yarn;")) + NSLOCTEXT("UYarnAssetFactory", "FormatTxt", "Yarn File").ToString());
    // Formats.Add(FString(TEXT("yarnc;")) + NSLOCTEXT("UYarnAssetFactory", "FormatTxt", "Compiled Yarn File").ToString());
    // Formats.Add(FString(TEXT("yarnproject;")) + NSLOCTEXT("UYarnAssetFactory", "FormatTxt", "Yarn Project").ToString());
    SupportedClass = UYarnAsset::StaticClass();
    bCreateNew = false;
    bEditorImport = true;
}

UObject* UYarnAssetFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
{
//    FEditorDelegates::OnAssetPreImport.Broadcast(this, InClass, InParent, InName, Type);
    
    UYarnAsset* YarnAsset = nullptr;
    FString TextString;

    YarnAsset = NewObject<UYarnAsset>(InParent, InClass, InName, Flags);
    // YarnAsset->SourceFilePath = UAssetImportData::SanitizeImportFilename(CurrentFilename, YarnAsset->GetOutermost());

    const TCHAR* fileName = *CurrentFilename;

	Yarn::CompilerOutput compilerOutput = UYarnAssetFactory::GetCompiledDataForScript(fileName);

	if (!compilerOutput.IsInitialized())
	{
		UE_LOG(LogYarnSpinnerEditor, Error, TEXT("Failed to get results from the compiler. Stopping import."));
		return nullptr;
	}

	bool anyErrors = false;
	for (auto diagnostic : compilerOutput.diagnostics())
	{
		if (diagnostic.severity() == Yarn::Diagnostic_Severity::Diagnostic_Severity_Error)
		{
			UE_LOG(LogYarnSpinnerEditor, Error, TEXT("Error: %s:%i %s"),
				   UTF8_TO_TCHAR(diagnostic.filename().c_str()),
				   diagnostic.range().start().line(),
				   UTF8_TO_TCHAR(diagnostic.message().c_str()));
			anyErrors = true;
		}
		else if (diagnostic.severity() == Yarn::Diagnostic_Severity::Diagnostic_Severity_Warning)
		{
			UE_LOG(LogYarnSpinnerEditor, Warning, TEXT("Warning: %s:%i %s"),
				   UTF8_TO_TCHAR(diagnostic.filename().c_str()),
				   diagnostic.range().start().line(),
				   UTF8_TO_TCHAR(diagnostic.message().c_str()));
		}
		else if (diagnostic.severity() == Yarn::Diagnostic_Severity::Diagnostic_Severity_Info)
		{
			UE_LOG(LogYarnSpinnerEditor, Log, TEXT("%s:%i %s"),
				   UTF8_TO_TCHAR(diagnostic.filename().c_str()),
				   diagnostic.range().start().line(),
				   UTF8_TO_TCHAR(diagnostic.message().c_str()));
		}
	}

	if (anyErrors || !compilerOutput.program().IsInitialized())
	{
		UE_LOG(LogYarnSpinnerEditor, Error, TEXT("File contains errors; stopping import."));
		return nullptr;
	}
	
	// Now convert that into binary wire format for saving
	std::string data = compilerOutput.program().SerializeAsString();

	// Finally, convert THAT into a TArray of bytes and we're done!
	TArray<uint8> output = TArray<uint8>((const uint8*)data.c_str(), data.size());

	YarnAsset->Data = output;

	// FString Result;
	// if (FFileHelper::LoadFileToArray(YarnAsset->Data, fileName)) {
	// 	// TODO: report successfully loading the data
	// } else {
	// 	// TODO: report failing to load the data
	// }

	// Record where this asset came from so we know how to update it
	if (!CurrentFilename.IsEmpty())
	{
        YarnAsset->AssetImportData->Update(CurrentFilename);
    }
    
//    FEditorDelegates::OnAssetPostImport.Broadcast(this, YarnAsset);

    return YarnAsset;
}

bool UYarnAssetFactory::FactoryCanImport(const FString& Filename) {
	// return FPaths::GetExtension(Filename).Equals(TEXT("yarnc"));
	return FPaths::GetExtension(Filename).Equals(TEXT("yarn"));
	//     || FPaths::GetExtension(Filename).Equals(TEXT("yarnproject"));
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

Yarn::CompilerOutput UYarnAssetFactory::GetCompiledDataForScript(const TCHAR* InFilePath) {
	FString yscPath = FPaths::Combine(FPaths::ProjectPluginsDir(), FString(YSC_PATH));

	FString stdOut;
	FString stdErr;

	int32 returnCode;

	FString params;

	params.Append(TEXT("compile "));
	params.Append(TEXT("--stdout "));
	params.Append(InFilePath);

	// Run ysc to get our compilation result
	UE_LOG(LogYarnSpinnerEditor, Log, TEXT("Calling ysc with %s"), *params);
	FPlatformProcess::ExecProcess(*yscPath, *params, &returnCode, &stdOut, &stdErr);

	UE_LOG(LogYarnSpinnerEditor, Log, TEXT("ysc returned %i; stdout:\n%s\nstderr:%s\n"), returnCode, *stdOut, *stdErr);

	Yarn::CompilerOutput compilerOutput;

	if (returnCode != 0) {
		UE_LOG(LogYarnSpinnerEditor, Error, TEXT("Error compiling Yarn script: %s"), *stdErr);
		return compilerOutput;
	}

	// Convert stdout from an FString to a std::string
	std::string json(TCHAR_TO_UTF8(*stdOut));

	// Parse the incoming JSON into a Program message (to check that it's valid)
	auto status = google::protobuf::util::JsonStringToMessage(json, &compilerOutput);

	if (!status.ok()) {
		// Whoa, we failed to parse a CompilerOutput struct from the compiler.
		UE_LOG(LogYarnSpinnerEditor, Error, TEXT("Error importing result from ysc: %s"), status.ToString().c_str());
		return compilerOutput;
	}

	return compilerOutput;
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
		
		// Always allow reimporting a yarn asset
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

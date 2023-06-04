// Fill out your copyright notice in the Description page of Project Settings.

#include "YarnAssetFactory.h"

#include "YarnSpinnerEditor.h"

#include "Misc/FileHelper.h"
#include "EditorFramework/AssetImportData.h"
#include "Containers/UnrealString.h"

#include "ReimportYarnAssetFactory.h"
#include "Misc/YSLogging.h"

THIRD_PARTY_INCLUDES_START
#include "YarnSpinnerCore/yarn_spinner.pb.h"
#include "YarnSpinnerCore/compiler_output.pb.h"

#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/type_resolver_util.h>
THIRD_PARTY_INCLUDES_END

// google::protobuf::Message &from_json(google::protobuf::Message &msg, const std::string &json);

UYarnAssetFactory::UYarnAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Formats.Add(FString(TEXT("yarnproject;")) + NSLOCTEXT("UYarnAssetFactory", "FormatTxt", "Yarn Project File").ToString());
	// Formats.Add(FString(TEXT("yarnc;")) + NSLOCTEXT("UYarnAssetFactory", "FormatTxt", "Compiled Yarn File").ToString());
	// Formats.Add(FString(TEXT("yarnproject;")) + NSLOCTEXT("UYarnAssetFactory", "FormatTxt", "Yarn Project").ToString());
	SupportedClass = UYarnProjectAsset::StaticClass();
	bCreateNew = false;
	bEditorImport = true;
}


UObject* UYarnAssetFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
{
	//    FEditorDelegates::OnAssetPreImport.Broadcast(this, InClass, InParent, InName, Type);

	UYarnProjectAsset* YarnAsset = nullptr;
	FString TextString;

	YarnAsset = NewObject<UYarnProjectAsset>(InParent, InClass, InName, Flags);
	// YarnAsset->SourceFilePath = UAssetImportData::SanitizeImportFilename(CurrentFilename, YarnAsset->GetOutermost());

	const TCHAR* FileName = *CurrentFilename;

	Yarn::CompilerOutput CompilerOutput;

	// Record where this asset came from so we know how to update it
	if (!CurrentFilename.IsEmpty())
	{
		YarnAsset->AssetImportData->Update(CurrentFilename);
	}

	bool bSuccess = GetCompiledDataForYarnProject(FileName, CompilerOutput);

	if (!bSuccess)
	{
		UE_LOG(LogYarnSpinnerEditor, Error, TEXT("Failed to get results from the compiler. Stopping import."));
		return nullptr;
	}

	bool bAnyErrors = false;
	for (auto Diagnostic : CompilerOutput.diagnostics())
	{
		if (Diagnostic.severity() == Yarn::Diagnostic_Severity::Diagnostic_Severity_Error)
		{
			UE_LOG(LogYarnSpinnerEditor, Error, TEXT("Error: %s:%i %s"),
					UTF8_TO_TCHAR(Diagnostic.filename().c_str()),
					Diagnostic.range().start().line(),
					UTF8_TO_TCHAR(Diagnostic.message().c_str()));
			bAnyErrors = true;
		}
		else if (Diagnostic.severity() == Yarn::Diagnostic_Severity::Diagnostic_Severity_Warning)
		{
			UE_LOG(LogYarnSpinnerEditor, Warning, TEXT("Warning: %s:%i %s"),
					UTF8_TO_TCHAR(Diagnostic.filename().c_str()),
					Diagnostic.range().start().line(),
					UTF8_TO_TCHAR(Diagnostic.message().c_str()));
		}
		else if (Diagnostic.severity() == Yarn::Diagnostic_Severity::Diagnostic_Severity_Info)
		{
			UE_LOG(LogYarnSpinnerEditor, Log, TEXT("%s:%i %s"),
					UTF8_TO_TCHAR(Diagnostic.filename().c_str()),
					Diagnostic.range().start().line(),
					UTF8_TO_TCHAR(Diagnostic.message().c_str()));
		}
	}

	if (bAnyErrors)
	{
		UE_LOG(LogYarnSpinnerEditor, Error, TEXT("File contains errors; stopping import."));
		return nullptr;
	}

	// Now convert the Program into binary wire format for saving
	std::string Data = CompilerOutput.program().SerializeAsString();

	// And convert THAT into a TArray of bytes for storage
	TArray<uint8> Output = TArray<uint8>((const uint8*)Data.c_str(), Data.size());

	YarnAsset->Data = Output;

	// For each line we've received, store it in the Yarn asset
	for (auto Pair : CompilerOutput.strings())
	{
		FName LineID = FName(Pair.first.c_str());
		FString LineText = FString(Pair.second.text().c_str());
		YarnAsset->Lines.Add(LineID, LineText);
	}

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

	// Store source file data on asset for future comparison
	TArray<FString> SourceFiles;
	bSuccess = GetSourcesForProject(FileName, SourceFiles);

	if (bSuccess)
	{
		YarnAsset->SetYarnSources(SourceFiles);
	}

	//    YarnAsset->PostEditChange();
	//    YarnAsset->MarkPackageDirty();

	//    FEditorDelegates::OnAssetPostImport.Broadcast(this, YarnAsset);

	return YarnAsset;
}


bool UYarnAssetFactory::FactoryCanImport(const FString& Filename)
{
	// return FPaths::GetExtension(Filename).Equals(TEXT("yarnc"));
	return FPaths::GetExtension(Filename).Equals(TEXT("yarn"));
	//     || FPaths::GetExtension(Filename).Equals(TEXT("yarnproject"));
}


EReimportResult::Type UYarnAssetFactory::Reimport(UYarnProjectAsset* TextAsset)
{
	auto Path = TextAsset->AssetImportData->GetFirstFilename();

	if (Path.IsEmpty() == false)
	{
		FString FilePath = IFileManager::Get().ConvertToRelativePath(*Path);

		TArray<uint8> Data;

		if (FFileHelper::LoadFileToArray(Data, *FilePath))
		{
			const uint8* Ptr = Data.GetData();
			CurrentFilename = FilePath; //not thread safe but seems to be how it is done..
			bool bWasCancelled = false;
			UObject* Result = FactoryCreateBinary(TextAsset->GetClass(), TextAsset->GetOuter(), TextAsset->GetFName(), TextAsset->GetFlags(), nullptr, *FPaths::GetExtension(FilePath), Ptr, Ptr + Data.Num(), GWarn);

			// if (bWasCancelled)
			// {
			// 	return EReimportResult::Cancelled;
			// }
			return Result ? EReimportResult::Succeeded : EReimportResult::Failed;
		}
	}
	return EReimportResult::Failed;
}


FString UYarnAssetFactory::YscPath()
{
	return FPaths::Combine(FPaths::ProjectPluginsDir(), FString(YSC_PATH));
}


bool UYarnAssetFactory::GetCompiledDataForYarnProject(const TCHAR* InFilePath, Yarn::CompilerOutput& CompilerOutput)
{
	FString StdOut;
	FString StdErr;

	int32 ReturnCode;

	const FString Params = FString::Printf(TEXT("compile --stdout %s"), InFilePath);

	// Run ysc to get our compilation result
	UE_LOG(LogYarnSpinnerEditor, Log, TEXT("Calling ysc with %s"), *Params);
	FPlatformProcess::ExecProcess(*YscPath(), *Params, &ReturnCode, &StdOut, &StdErr);

	UE_LOG(LogYarnSpinnerEditor, Log, TEXT("ysc returned %i;"), ReturnCode);
	UE_LOG(LogYarnSpinnerEditor, Log, TEXT("stdout:"));
	YS_LOG_CLEAN("%s", *StdOut);
	UE_LOG(LogYarnSpinnerEditor, Log, TEXT("stderr:"));
	YS_LOG_CLEAN("%s", *StdErr);

	if (ReturnCode != 0)
	{
		UE_LOG(LogYarnSpinnerEditor, Error, TEXT("Error compiling Yarn script: %s"), *StdErr);
		return false;
	}

	// Convert stdout from an FString to a std::string
	const std::string JSON(TCHAR_TO_UTF8(*StdOut));

	// Parse the incoming JSON into a Program message (to check that it's valid)
	const auto Status = google::protobuf::util::JsonStringToMessage(JSON, &CompilerOutput);

	if (!Status.ok())
	{
		// Whoa, we failed to parse a CompilerOutput struct from the compiler.
		UE_LOG(LogYarnSpinnerEditor, Error, TEXT("Error importing result from ysc: %s"), *FString(Status.ToString().c_str()));

		return false;
	}

	return true;
}


bool UYarnAssetFactory::GetSourcesForProject(const TCHAR* InFilePath, TArray<FString>& SourceFiles)
{
	// Run ysc again to get the list of source yarn files used in the compilation
	FString StdOut;
	FString StdErr;
	int32 ReturnCode;

	const FString Params = FString::Printf(TEXT("list-sources %s"), InFilePath);
	FPlatformProcess::ExecProcess(*YscPath(), *Params, &ReturnCode, &StdOut, &StdErr);

	if (ReturnCode != 0)
	{
		YS_ERR("Error getting .yarn source list: %s", *StdErr);
		return false;
	}

	StdOut.ParseIntoArrayLines(SourceFiles);
	return true;
}


bool UYarnAssetFactory::GetSourcesForProject(const UYarnProjectAsset* YarnProjectAsset, TArray<FString>& SourceFiles)
{
	if (!YarnProjectAsset->AssetImportData)
	{
		YS_ERR("YarnProjectAsset has no AssetImportData");
		return false;
	}
	return GetSourcesForProject(*YarnProjectAsset->AssetImportData->GetFirstFilename(), SourceFiles);
}


///////////// Reimport

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
	UYarnProjectAsset* DataTable = Cast<UYarnProjectAsset>(Obj);
	if (DataTable)
	{
		DataTable->AssetImportData->ExtractFilenames(OutFilenames);

		// Always allow reimporting a yarn asset
		return true;
	}
	return false;
}


void UReimportYarnAssetFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UYarnProjectAsset* DataTable = Cast<UYarnProjectAsset>(Obj);
	if (DataTable && ensure(NewReimportPaths.Num() == 1))
	{
		DataTable->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}


EReimportResult::Type UReimportYarnAssetFactory::Reimport(UObject* Obj)
{
	EReimportResult::Type Result = EReimportResult::Failed;
	if (UYarnProjectAsset* DataTable = Cast<UYarnProjectAsset>(Obj))
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

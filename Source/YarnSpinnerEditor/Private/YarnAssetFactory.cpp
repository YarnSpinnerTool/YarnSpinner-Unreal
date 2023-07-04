// Fill out your copyright notice in the Description page of Project Settings.

#include "YarnAssetFactory.h"

#include "LocalizationSourceControlUtil.h"
#include "LocTextHelper.h"
#include "YarnSpinnerEditor.h"

#include "Misc/FileHelper.h"
#include "EditorFramework/AssetImportData.h"
#include "Containers/UnrealString.h"

#include "ReimportYarnAssetFactory.h"
#include "YarnProjectMeta.h"
#include "Misc/YSLogging.h"
#include "Serialization/Csv/CsvParser.h"

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
    SupportedClass = UYarnProject::StaticClass();
    bCreateNew = false;
    bEditorImport = true;
}


UObject* UYarnAssetFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext* Warn)
{
    YS_LOG_FUNCSIG
    
    //    FEditorDelegates::OnAssetPreImport.Broadcast(this, InClass, InParent, InName, Type);

    UYarnProject* YarnProject = nullptr;
    FString TextString;

    YarnProject = NewObject<UYarnProject>(InParent, InClass, InName, Flags);
    // YarnAsset->SourceFilePath = UAssetImportData::SanitizeImportFilename(CurrentFilename, YarnAsset->GetOutermost());

    const TCHAR* FileName = *CurrentFilename;

    Yarn::CompilerOutput CompilerOutput;

    // Record where this asset came from so we know how to update it
    if (!CurrentFilename.IsEmpty())
    {
        YarnProject->AssetImportData->Update(CurrentFilename);
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

    YarnProject->Data = Output;

    // For each line we've received, store it in the Yarn asset
    for (auto Pair : CompilerOutput.strings())
    {
        FName LineID = FName(Pair.first.c_str());
        FString LineText = FString(Pair.second.text().c_str());
        YarnProject->Lines.Add(LineID, LineText);
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
        YarnProject->AssetImportData->Update(CurrentFilename);
    }

    // Store source file data on asset for future comparison
    TArray<FString> SourceFiles;
    bSuccess = GetSourcesForProject(FileName, SourceFiles);

    if (bSuccess)
    {
        YarnProject->SetYarnSources(SourceFiles);
    }

    
    // Set up localisation target for the Yarn project

    TOptional<FYarnProjectMetaData> ProjectMeta = FYarnProjectMetaData::FromAsset(YarnProject);
    if (!ProjectMeta.IsSet())
    {
        YS_ERR("Failed to get project metadata from .yarnproject file.  Could not create/update localisation target.");
        return YarnProject;
    }

	const FString LocTargetName = InName.ToString();
	const FString LocTargetPath = FPaths::ProjectContentDir() / TEXT("Localization") / LocTargetName;

    TArray<FString> Cultures;
    ProjectMeta->localisation.GetKeys(Cultures);
    
	FLocTextHelper LocTextHelper(LocTargetPath, FString::Printf(TEXT("%s.manifest"), *LocTargetName), FString::Printf(TEXT("%s.archive"), *LocTargetName), ProjectMeta->baseLanguage, Cultures, nullptr);

    // TODO: set up localisation target properties
    
    // TODO: look at ULocalisationTarget and FLocalisationTargetSettings ...
    
    // TODO: fill out manifest
    // TODO: create language archives

    FText OutError;
    // if (!LocTextHelper.LoadManifest(ELocTextHelperLoadFlags::LoadOrCreate, &OutError))
    if (!LocTextHelper.LoadAll(ELocTextHelperLoadFlags::Create, &OutError))
    {
        YS_ERR("Could not create manifest & archive files for localisation target '%s': %s", *LocTargetName, *OutError.ToString());
    }
    else
    {
        // FString Desc = "A description";
        FLocKey NamespaceKey {LocTargetName};
        // FManifestContext ManifestContext {FLocKey("line1234")};
        // ManifestContext.SourceLocation = YarnProject->GetPathName();
        // TSharedPtr<FLocMetadataObject> LocMetadataObject {MakeShared<FLocMetadataObject>()};
        // TSharedPtr<FLocMetadataValueString> LocMetadataValue {MakeShared<FLocMetadataValueString>(" AAAA STRRRINNNGNGG ")};
        // LocMetadataObject->SetField("THEFIELDNAME", LocMetadataValue );
        // FLocItem Item { "Line1234", LocMetadataObject};
        // FLocItem Item { "Default text" };
        // LocTextHelper.AddSourceText(NamespaceKey, Item, ManifestContext);//, &Desc);
        
        // Add lines to manifest as source text
        for (auto Pair : CompilerOutput.strings())
        {
            FString LineID = FString(Pair.first.c_str());
            FString LineText = FString(Pair.second.text().c_str());
            FManifestContext ManifestContext {FLocKey(LineID)};
            ManifestContext.SourceLocation = YarnProject->GetPathName();
            LocTextHelper.AddSourceText(NamespaceKey, FLocItem(LineText), ManifestContext);
        }
        
        // Add translations to archives
        for (auto Loc : ProjectMeta->localisation)
        {
            FString Culture = Loc.Key;
            FYarnProjectLocalizationData LocData = Loc.Value;
            FString LocFile = FPaths::Combine(YarnProject->YarnProjectPath(), LocData.strings);
            FPaths::NormalizeFilename(LocFile);
            FPaths::CollapseRelativeDirectories(LocFile);
            FPaths::RemoveDuplicateSlashes(LocFile);
            FString LocFileData;
            
            if (!FFileHelper::LoadFileToString(LocFileData, *LocFile))
            {
                YS_WARN("Couldn't load strings file: %s", *LocFile)
                continue;
            }

            const FCsvParser Parser(LocFileData);
            const FCsvParser::FRows& Rows = Parser.GetRows();
            if (Rows.Num() < 2)
            {
                YS_WARN("Empty strings file: %s", *LocFile)
                continue;
            }
            
            const auto& HeaderRow = Rows[0];
            TMap<FString, int32> HeaderMap;
            for (int32 I = 0; I < HeaderRow.Num(); ++I)
            {
                HeaderMap.Add(WCHAR_TO_TCHAR(HeaderRow[I]), I);
            }
            // Test for required columns
            if (!HeaderMap.Contains(TEXT("character")) || !HeaderMap.Contains(TEXT("text")) || !HeaderMap.Contains(TEXT("id")))
            {
                YS_ERR("Missing required column 'id', 'text' or 'character' in strings file: %s", *LocFile)
                continue;
            }

            for (int32 I = 1; I < Rows.Num(); ++I)
            {
                const auto& Row = Rows[I];
                const FString& LineID = WCHAR_TO_TCHAR(Row[HeaderMap[TEXT("id")]]);
                const FString& LineText = WCHAR_TO_TCHAR(Row[HeaderMap[TEXT("text")]]);
                const FString& LineCharacter = WCHAR_TO_TCHAR(Row[HeaderMap[TEXT("character")]]);

                auto Source = LocTextHelper.FindSourceText(NamespaceKey, FLocKey(LineID));

                FLocItem SourceText = Source.IsValid() ? Source->Source : FLocItem();
                FLocItem Translation((!LineCharacter.IsEmpty() ? LineCharacter + TEXT(": ") : TEXT("")) + LineText);

                const auto LocEntry = MakeShared<FArchiveEntry>(NamespaceKey, FLocKey(LineID), SourceText, Translation, nullptr, false);

                LocTextHelper.AddTranslation(Culture, LocEntry);
            }
        }

        LocTextHelper.SaveAll();
        
        // TODO: compile
    }





    //    YarnAsset->PostEditChange();
    //    YarnAsset->MarkPackageDirty();

    //    FEditorDelegates::OnAssetPostImport.Broadcast(this, YarnAsset);

    return YarnProject;
}


bool UYarnAssetFactory::FactoryCanImport(const FString& Filename)
{
    // return FPaths::GetExtension(Filename).Equals(TEXT("yarnc"));
    // return FPaths::GetExtension(Filename).Equals(TEXT("yarn"));
    return FPaths::GetExtension(Filename).Equals(TEXT("yarnproject"));
}


EReimportResult::Type UYarnAssetFactory::Reimport(UYarnProject* YarnProject)
{
    YS_LOG_FUNCSIG
    
    const FString Path = YarnProject->AssetImportData->GetFirstFilename();

    if (Path.IsEmpty() == false)
    {
        const FString FilePath = IFileManager::Get().ConvertToRelativePath(*Path);

        TArray<uint8> Data;

        if (FFileHelper::LoadFileToArray(Data, *FilePath))
        {
            const uint8* Ptr = Data.GetData();
            CurrentFilename = FilePath; //not thread safe but seems to be how it is done..
            bool bWasCancelled = false;
            UYarnProject* Result = Cast<UYarnProject>(FactoryCreateBinary(YarnProject->GetClass(), YarnProject->GetOuter(), YarnProject->GetFName(), YarnProject->GetFlags(), nullptr, *FPaths::GetExtension(FilePath), Ptr, Ptr + Data.Num(), GWarn));

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


bool UYarnAssetFactory::GetSourcesForProject(const UYarnProject* YarnProjectAsset, TArray<FString>& SourceFiles)
{
    if (!YarnProjectAsset->AssetImportData)
    {
        YS_ERR("YarnProjectAsset has no AssetImportData");
        return false;
    }
    return GetSourcesForProject(*YarnProjectAsset->AssetImportData->GetFirstFilename(), SourceFiles);
}

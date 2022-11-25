// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "YarnSpinnerEditor.h"


#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "YarnAsset.h"
#include "EditorReimportHandler.h"

#include "compiler_output.pb.h"

#include "YarnAssetFactory.generated.h"


/**
 * 
 */
UCLASS(hidecategories=Object)
class UYarnAssetFactory : public UFactory
{
    GENERATED_UCLASS_BODY()
    
public:
    virtual UObject* FactoryCreateBinary
 (
     UClass* InClass,
     UObject* InParent,
     FName InName,
     EObjectFlags Flags,
     UObject* Context,
     const TCHAR* Type,
     const uint8*& Buffer,
     const uint8* BufferEnd,
     FFeedbackContext* Warn
 ) override;
    
    virtual bool FactoryCanImport(const FString& Filename) override;

    EReimportResult::Type Reimport(UYarnAsset* TextAsset);

    static bool GetCompiledDataForScript(const TCHAR *InFilePath, Yarn::CompilerOutput &InCompilerOutput);
};

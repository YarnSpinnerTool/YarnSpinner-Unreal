﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/YarnCommandLibrary.h"

#include "DialogueRunner.h"
#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Library/YarnLibraryRegistry.h"
#include "Misc/OutputDeviceNull.h"
#include "Misc/YSLogging.h"
#include "UObject/SoftObjectPtr.h"



// Sets default values
UYarnCommandLibrary::UYarnCommandLibrary()
{
    YS_LOG_FUNCSIG
    
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    // PrimaryActorTick.bCanEverTick = true;

    // Attempt to create subobjects from Blueprint-defined Blueprint Function Libraries

    // FARFilter Filter;
    // // Filter.PackageNames.Add(TEXT("/Game/NewCommandLibrary"));
    // Filter.PackageNames.Add(TEXT("/Game"));
    // TArray<FAssetData> AssetData;
    // FAssetRegistryModule::GetRegistry().GetAssets(Filter, AssetData);
    // if (AssetData.Num() == 0)
    // {
    //     YS_WARN_FUNC("Asset search found nothing")
    // }
    // for (auto Asset : AssetData)
    // {
    //     // CreateDefaultSubobject<UWidgetComponent>()
    //     
    //     YS_LOG_FUNC("Found asset: %s", *Asset.AssetName.ToString())
    //     // Asset.GetClass()->CreateDefaultSubobject<UBlueprintCommandLibrary>()
    //     // Asset.GetClass()->
    //     // NewObject<UBlueprintCommandLibrary>(Asset.GetClass());
    //     // CreateDefaultSubobject<UBlueprintCommandLibrary>(Asset.AssetName, false);
    //     // Create
    //
    //     // NewObject
    //     
    // }
}


UYarnCommandLibrary* UYarnCommandLibrary::FromBlueprint(const UBlueprint* Blueprint)
{
    if (!Blueprint || !Blueprint->GeneratedClass || !Blueprint->GeneratedClass->GetDefaultObject())
        return nullptr;

    return Cast<UYarnCommandLibrary>(Blueprint->GeneratedClass->GetDefaultObject());
}


void UYarnCommandLibrary::CallCommand(FName CommandName, TSoftObjectPtr<ADialogueRunner> DialogueRunner, TArray<FYarnBlueprintParam> Args)
{
    // Find the function
    UFunction* Function = FindFunction(CommandName);

    if (!Function)
    {
        YS_WARN("Could not find command function '%s'", *CommandName.ToString())
        return ContinueDialogue(DialogueRunner);
    }

    // Set up the parameters
    FStructOnScope FuncParams(Function);
    FBoolProperty* BoolParam;
    FFloatProperty* FloatParam;
    FStrProperty* StringParam;

    const FName DialogueRunnerField = TEXT("DialogueRunner");

    const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Function->FindPropertyByName(DialogueRunnerField));
    ObjectProperty->SetObjectPropertyValue_InContainer(FuncParams.GetStructMemory(), DialogueRunner.Get());

    // FSoftObjectProperty* DialogueRunnerProperty = CastField<FSoftObjectProperty>(Function->FindPropertyByName(DialogueRunnerField));
    // // FSoftObjectProperty SoftObjectProperty = FSoftObjectProperty
    // DialogueRunnerProperty->SetObjectPropertyValue_InContainer(FuncParams.GetStructMemory(), DialogueRunner);
    // DialogueRunnerProperty->SetPropertyValue_InContainer(FuncParams.GetStructMemory(), {DialogueRunner});

    for (auto Arg : Args)
    {
        // Set input properties
        switch (Arg.Value.GetType())
        {
        case Yarn::Value::ValueType::BOOL:
            BoolParam = CastField<FBoolProperty>(Function->FindPropertyByName(Arg.Name));
            if (!BoolParam)
            {
                YS_WARN_FUNC("Could not create function parameter '%s' for command '%s' from given values", *Arg.Name.ToString(), *CommandName.ToString())
                return ContinueDialogue(DialogueRunner);
            }
            BoolParam->SetPropertyValue_InContainer(FuncParams.GetStructMemory(), Arg.Value.GetBooleanValue());
            break;
        case Yarn::Value::ValueType::NUMBER:
            FloatParam = CastField<FFloatProperty>(Function->FindPropertyByName(Arg.Name));
            if (!FloatParam)
            {
                YS_WARN_FUNC("Could not create function parameter '%s' for command '%s' from given values", *Arg.Name.ToString(), *CommandName.ToString())
                return ContinueDialogue(DialogueRunner);
            }
            FloatParam->SetPropertyValue_InContainer(FuncParams.GetStructMemory(), Arg.Value.GetNumberValue());
            break;
        case Yarn::Value::ValueType::STRING:
            StringParam = CastField<FStrProperty>(Function->FindPropertyByName(Arg.Name));
            if (!StringParam)
            {
                YS_WARN_FUNC("Could not create function parameter '%s' for command '%s' from given values", *Arg.Name.ToString(), *CommandName.ToString())
                return ContinueDialogue(DialogueRunner);
            }
            StringParam->SetPropertyValue_InContainer(FuncParams.GetStructMemory(), FString(Arg.Value.GetStringValue().c_str()));
            break;
        }
    }

    // Call the function (and assume it correctly calls Continue on the DialogueRunner)
    ProcessEvent(Function, FuncParams.GetStructMemory());
}


void UYarnCommandLibrary::ContinueDialogue(const TSoftObjectPtr<ADialogueRunner>& DialogueRunner)
{
    if (DialogueRunner.IsValid())
        DialogueRunner->ContinueDialogue();
}



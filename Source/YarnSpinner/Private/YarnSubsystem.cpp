﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "YarnSubsystem.h"

#include "Engine/ObjectLibrary.h"
#include "Library/YarnCommandLibrary.h"
#include "Library/YarnFunctionLibrary.h"
#include "Library/YarnLibraryRegistry.h"
#include "Misc/YarnAssetHelpers.h"
#include "Misc/YSLogging.h"


UYarnSubsystem::UYarnSubsystem()
{
    YS_LOG_FUNCSIG

    // TODO: move to editor?
    YarnFunctionObjectLibrary = UObjectLibrary::CreateLibrary(UYarnFunctionLibrary::StaticClass(), true, true);
    YarnCommandObjectLibrary = UObjectLibrary::CreateLibrary(UYarnCommandLibrary::StaticClass(), true, true);
    YarnFunctionObjectLibrary->AddToRoot();
    YarnCommandObjectLibrary->AddToRoot();
    YarnFunctionObjectLibrary->bRecursivePaths = true;
    YarnCommandObjectLibrary->bRecursivePaths = true;
    YarnFunctionObjectLibrary->LoadAssetDataFromPath(TEXT("/Game"));
    YarnCommandObjectLibrary->LoadAssetDataFromPath(TEXT("/Game"));

    YarnFunctionRegistry = NewObject<UYarnLibraryRegistry>(this, "YarnFunctionRegistry");
}


void UYarnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    YS_LOG_FUNCSIG
    Super::Initialize(Collection);


    TArray<TSubclassOf<UYarnFunctionLibrary>> LibRefs;
    TArray<FAssetData> Blueprints;

    FARFilter Filter;
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 3
    Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
    Filter.ClassPaths.Add(UBlueprintGeneratedClass::StaticClass()->GetClassPathName());
#else
    Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
    Filter.ClassNames.Add(UBlueprintGeneratedClass::StaticClass()->GetFName());
#endif
    FAssetRegistryModule::GetRegistry().GetAssets(Filter, Blueprints);

    for (auto Asset : Blueprints)
    {
        if (UObject* BPObj = Cast<UObject>(Asset.GetAsset()))
        {
            UBlueprint* BP = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *BPObj->GetPathName()));
            if (BP && BP->GeneratedClass && BP->GeneratedClass->GetDefaultObject())
            {
                // if (BP->GetBlueprintClass()->IsChildOf<AYarnFunctionLibrary>())
                if (auto YFL = UYarnFunctionLibrary::FromBlueprint(BP))
                {
                    YS_LOG_FUNC("FOUNDDDD OOOONNNNEEE")
                    LibRefs.Add(*BP->GeneratedClass);
                }
            }
        }
    }

    for (auto Lib : LibRefs)
    {
        if (Lib->FindFunctionByName(FName("MyQuickActorFunction")))
        {
            YS_LOG_FUNC("FOUND MyQuickActorFunction")
            auto YFL = Cast<UYarnFunctionLibrary>(Lib->GetDefaultObject());
            auto Result1 = YFL->CallFunction("MyQuickActorFunction", {FYarnBlueprintParam{"InParam", Yarn::FValue(12.345)}}, {{"OutParam", Yarn::FValue(true)}});
            auto Result2 = YFL->CallFunction("MyQuickActorFunction", {FYarnBlueprintParam{"InParam", Yarn::FValue(1234.5)}}, {{"OutParam", Yarn::FValue(true)}});

            YS_LOG_FUNC("Did we succeed? %d, %d", Result1.IsSet() && Result1->GetValue<bool>(), Result2.IsSet() && Result2->GetValue<bool>())
        }
        if (Lib->FindFunctionByName(FName("MyAwesomeFunc")))
        {
            YS_LOG_FUNC("calling YarnFunctionLibrary %s->MyAwesomeFunc()", *Lib->GetName())
            auto YFL = Cast<UYarnFunctionLibrary>(Lib->GetDefaultObject());
            auto Result = YFL->CallFunction("MyAwesomeFunc", {FYarnBlueprintParam{"FirstInParam", Yarn::FValue(true)}, FYarnBlueprintParam{"SecondInParam", Yarn::FValue(12.345)}}, {{"OutParam", Yarn::FValue(0.0)}});
            if (Result.IsSet())
            {
                YS_LOG_FUNC("Function returned: %f", Result->GetValue<double>())
            }
        }
    }

}


void UYarnSubsystem::Deinitialize()
{
    YS_LOG_FUNCSIG
    Super::Deinitialize();
}


void UYarnSubsystem::SetValue(const FString& name, bool value)
{
    Variables.FindOrAdd(name) = value;
}


void UYarnSubsystem::SetValue(const FString& name, float value)
{
    Variables.FindOrAdd(name) = value;
}


void UYarnSubsystem::SetValue(const FString& name, const FString& value)
{
    Variables.FindOrAdd(name) = value;
}


bool UYarnSubsystem::HasValue(const FString& name)
{
    return Variables.Contains(name);
}


Yarn::FValue UYarnSubsystem::GetValue(const FString& name)
{
    return Variables.FindOrAdd(name);
}


void UYarnSubsystem::ClearValue(const FString& name)
{
    Variables.Remove(name);
}





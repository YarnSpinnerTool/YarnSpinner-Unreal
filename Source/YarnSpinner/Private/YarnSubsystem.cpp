// Fill out your copyright notice in the Description page of Project Settings.


#include "YarnSubsystem.h"

#include "DisplayLine.h"
#include "YarnFunctionLibrary.h"
#include "Misc/OutputDeviceNull.h"
#include "Misc/YarnAssetHelpers.h"
#include "Misc/YSLogging.h"


void UYarnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    YS_LOG_FUNCSIG
    Super::Initialize(Collection);

    if (FAssetRegistryModule::GetRegistry().IsLoadingAssets())
    {
        YS_WARN_FUNC("Asset registry still loading assets...")
    }

    FARFilter Filter;
    // Filter.PackageNames.Add(TEXT("NewFunctionLibrary"));
    // Filter.PackageNames.Add(TEXT("/Game/NewFunctionLibrary.NewFunctionLibrary"));
    // Filter.PackageNames.Add(TEXT("D:/dev/YarnSpinner/YSUEDemo4.27/Content/NewFunctionLibrary.uasset"));
    // Filter.PackageNames.Add(TEXT("/Game/NewFunctionLibrary"));
    Filter.PackageNames.Add(TEXT("/Game"));
    TArray<FAssetData> AssetData;
    FAssetRegistryModule::GetRegistry().GetAssets(Filter, AssetData);
    if (AssetData.Num() == 0)
    {
        YS_WARN_FUNC("Asset search found nothing")
    }

    // TODO: get the blueprint library
    // auto Assets = FYarnAssetHelpers::FindAssetsInRegistryByPackageName<UBlueprint>(TEXT("/Game/NewFunctionLibrary"));
    auto Assets = FYarnAssetHelpers::FindAssetsInRegistryByPackageName<UBlueprint>(TEXT("/Game"));
    if (Assets.Num() == 0)
    {
        YS_WARN_FUNC("No blueprint library found")
    }

    for (auto BP : Assets)
    {
        TArray<FName> FunctionNames;
        BP.GetClass()->GenerateFunctionList(FunctionNames);

        YS_LOG_FUNC("Found blueprint: %s", *BP.AssetName.ToString())
        for (auto Func : FunctionNames)
        {
            YS_LOG_FUNC("Found function: %s", *Func.ToString())
        }

        auto Blueprint = Cast<UBlueprint>(BP.GetAsset());
        YS_LOG_FUNC("Calling blueprint function...")
        // auto Ar = FStringOutputDevice();
        // if (!CallFunctionByNameWithArguments(TEXT("MyLibFunction \"blah\" \"woo\""), Ar, nullptr, true))
        auto Ar = FOutputDeviceNull();
        // if (!Blueprint->CallFunctionByNameWithArguments(TEXT("MyLibFunction 11"), Ar, nullptr, true))

        // auto MyBlueprint = NewObject<UBlueprint>(this);//, Blueprint->GetClass());
        // auto MyBlueprint = NewObject<UBlueprintGeneratedClass*>(Blueprint->GetOuter(), Blueprint->GetBlueprintClass());//, Blueprint->GetClass());
        //
        // if (MyBlueprint->CallFunctionByNameWithArguments(TEXT("MyLibFunction 123"), Ar, nullptr, true))
        // {
        //     YS_LOG("Blueprint function call succeeded #1")
        // }

        if (BP.GetAsset()->CallFunctionByNameWithArguments(TEXT("MyLibFunction 123"), Ar, nullptr, true))
        {
            YS_LOG_FUNC("Blueprint function call succeeded #1")
        }

        YS_LOG_FUNC("Blueprint found: %s", *Blueprint->GetName());


        if (!Blueprint->CallFunctionByNameWithArguments(TEXT("MyLibFunction 123"), Ar, nullptr, true))
        {
            YS_LOG_FUNC("Blueprint function call failed")
        }
        else
        {
            YS_LOG_FUNC("Blueprint function call succeeded #2")
        }

        if (!Blueprint->FindFunction(FName("MyLibFunction")))
        {
            YS_LOG_FUNC("Function not found")
        }
        if (Blueprint->GeneratedClass->CallFunctionByNameWithArguments(TEXT("MyLibFunction"), Ar, nullptr, true))
        {
            YS_LOG_FUNC("WORKEDDD")
        }
    }

    FWorldDelegates::OnWorldInitializedActors.AddLambda([this](const UWorld::FActorsInitializedParams& Params) //UWorld* World, const UWorld::InitializationValues IVS)
    {
        YS_LOG_FUNC("World initialized")
        // YarnFunctionLibrary = GetWorld()->SpawnActor<AYarnFunctionLibrary>();
        // Blueprint'/Game/NewYarnFunctionLibrary.NewYarnFunctionLibrary'
        // Load a blueprint class (in a constructor only)
        // static ConstructorHelpers::FObjectFinder<UClass> QuickTestBP(TEXT("Blueprint'/Game/NewYarnFunctionLibrary.NewYarnFunctionLibrary'"));
        // if (QuickTestBP.Succeeded() && QuickTestBP.Object)
        // {
        //     YS_LOG_FUNC("Creating actor from blueprint")
        //     AActor* QuickTestActorFromBlueprint =
        //         GetWorld()->SpawnActor<AActor>(QuickTestBP.Object);
        // }
        // else
        // {
        //     YS_WARN_FUNC("Could not find blueprint class to create yarnfunction actor")
        // }
    });

    // YS_LOG("function output device: %s", *Ar)
}


void UYarnSubsystem::Deinitialize()
{
    YS_LOG_FUNCSIG
    Super::Deinitialize();
}


void UYarnSubsystem::SetValue(std::string name, bool value)
{
    Variables.FindOrAdd(FString(UTF8_TO_TCHAR(name.c_str()))) = Yarn::Value(value);
}


void UYarnSubsystem::SetValue(std::string name, float value)
{
    Variables.FindOrAdd(FString(UTF8_TO_TCHAR(name.c_str()))) = Yarn::Value(value);
}


void UYarnSubsystem::SetValue(std::string name, std::string value)
{
    Variables.FindOrAdd(FString(UTF8_TO_TCHAR(name.c_str()))) = Yarn::Value(value);
}


bool UYarnSubsystem::HasValue(std::string name)
{
    return Variables.Contains(FString(UTF8_TO_TCHAR(name.c_str())));
}


Yarn::Value UYarnSubsystem::GetValue(std::string name)
{
    return Variables.FindOrAdd(FString(UTF8_TO_TCHAR(name.c_str())));
}


void UYarnSubsystem::ClearValue(std::string name)
{
    Variables.Remove(FString(UTF8_TO_TCHAR(name.c_str())));
}

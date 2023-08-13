﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "YarnSubsystem.h"

#include "DisplayLine.h"
#include "YarnFunctionLibrary.h"
#include "Misc/OutputDeviceHelper.h"
#include "Misc/OutputDeviceNull.h"
#include "Misc/YarnAssetHelpers.h"
#include "Misc/YSLogging.h"


void UYarnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    YS_LOG_FUNCSIG
    Super::Initialize(Collection);

    TArray<FAssetData> ExistingAssets = FYarnAssetHelpers::FindAssetsInRegistryByPackagePath<UBlueprint>(FPaths::GetPath("/Game/"));
    YS_LOG_FUNC("Found %d assets using asset hlepers", ExistingAssets.Num())
    for (auto Asset : ExistingAssets)
    {
        YS_LOG_CLEAN("%s", *Asset.GetFullName())
        auto Lib = Cast<UBlueprint>(Asset.GetAsset());
        // auto Func = Lib->FindFunction(FName("NewFunction_0"));
        auto Func = Lib->GetClass()->FindFunctionByName(FName("NewFunction_0"));
        if (Func)
        {
            YS_LOG_FUNC("Found function: %s", *Func->GetName())
        }
        FOutputDeviceNull OutDevice;
        Lib->CallFunctionByNameWithArguments(TEXT("NewFunction_0"), OutDevice, nullptr);
        if (Lib->GetBlueprintClass()->FindFunctionByName(FName("NewFunction_0")))
        {
            YS_LOG_FUNC("FOUND ITTTTTTTTT")
        }

        // if (Asset)
        if (Lib->ParentClass->IsChildOf<AYarnFunctionLibrary>())
        {
            YS_LOG_FUNC("IS A YARN FUNCTION LIB")
            // TSubclassOf<AYarnFunctionLibrary> BPA = NewObject<AYarnFunctionLibrary>(this, Lib->GetBlueprintClass()));
            // TSubclassOf<AYarnFunctionLibrary> BPA = NewObject<AYarnFunctionLibrary>(this, Lib->OriginalClass));
            // auto BPA = NewObject<TSubclassOf<AYarnFunctionLibrary>>(this, Lib->GetClass());
            // if (BPA)
            // {
            //     YS_LOG_FUNC("got a BPA")
            // }

            // Lib->getblueprint
            
            //TSubclassOf<AYarnFunctionLibrary> BPA = Lib->GeneratedClass->GetDefaultObject<UBlueprintGeneratedClass>();
            //TSubclassOf<UBlueprintGeneratedClass> BPA = Lib->GeneratedClass->GetDefaultObject<UBlueprintGeneratedClass>();
            auto BPA = Lib->GetBlueprintClass()->GetDefaultObject<UBlueprintGeneratedClass>();
            if (BPA)
            {
                YS_LOG_FUNC("got a BPA")
                // if (BPA->FindFunctionByName(FName("NewFunction_0")))
                if (BPA->FindFunctionByName(FName("MyQuickActorFunction")))
                {
                    YS_LOG_FUNC("FOUND ITTTTTTTTT")
                }
                // BPA->GenerateFunctionList()
            }
            else
            {
                YS_LOG_FUNC("NO BPA")
            }
            // AYarnFunctionLibrary* YFL = Cast<AYarnFunctionLibrary>(Lib->GetBlueprintClass());
            // AYarnFunctionLibrary* YFL = Cast<AYarnFunctionLibrary>(Lib->GeneratedClass);
            // AYarnFunctionLibrary* YFL = Cast<AYarnFunctionLibrary>(BPA);
            // AYarnFunctionLibrary* YFL = Cast<AYarnFunctionLibrary>(Lib);
            AYarnFunctionLibrary* YFL = Cast<AYarnFunctionLibrary>(Lib->StaticClass());
            if (YFL)
            {
                YS_LOG_FUNC("ok, we created a YFL.... ..... asdfjlaskfjladskl")
            }
            // auto BP = ConstructorHelpers::FObjectFinder(Lib->GetPathName());
            // auto BP = ConstructorHelpers::FObjectFinder<UObject>(Lib->GetPathName());
            // auto BP = ConstructorHelpers::FObjectFinder<UBlueprint>(*Lib->GetPathName());
            FStringAssetReference BPRef(Lib->GetPathName());
            // if (BP.Succeeded())
            if (auto BP = BPRef.TryLoad())
            {
                YFL = Cast<AYarnFunctionLibrary>(BP);
                if (YFL)
                {
                    YS_LOG_FUNC("ok, we created a YFL.... ..... asdfjlaskfjladskl")
                }
            }
        }
        if (Lib->IsA<AYarnFunctionLibrary>())
        {
            YS_LOG_FUNC("IS A YARN FUNCTION LIB")
        }
    }

    return;
    
    const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    OnAssetRegistryFilesLoadedHandle = AssetRegistry.OnFilesLoaded().AddUObject(this, &UYarnSubsystem::OnAssetRegistryFilesLoaded);
    
    TArray<FAssetData> AllAssets;
    FAssetRegistryModule::GetRegistry().GetAllAssets(AllAssets, true);
    YS_LOG_FUNC("Found %d assets from GetAllAssets", AllAssets.Num())
    for (auto Asset : AllAssets)
    {
        // YS_LOG_CLEAN("%s", *Asset.GetFullName())
    }

    TArray<FAssetData> AssetsByPath;
    FAssetRegistryModule::GetRegistry().GetAssetsByPath(TEXT("/Game"), AssetsByPath, true);
    YS_LOG_FUNC("Found %d assets from GetAssetsByPath(\"/Game\")", AssetsByPath.Num())
    for (auto Asset : AssetsByPath)
    {
        YS_LOG_CLEAN("%s", *Asset.GetFullName())
    }

    TArray<FAssetData> ClassAssets;
    // AssetRegistryModule.Get().GetAssetsByClass(AYarnFunctionLibrary::StaticClass()->GetFName(),ClassAssets,true);
    AssetRegistry.GetAssetsByClass(FName("YarnFunctionLibrary"),ClassAssets,true);
    YS_LOG_FUNC("Found %d assets from GetAssetsByClass", ClassAssets.Num())

    TArray<FAssetData> AssetData;
    FARFilter Filter;
    AssetRegistry.GetAssets(Filter, AssetData);
    YS_LOG_FUNC("Found %d assets with empty/default filter", AssetData.Num())

    if (FAssetRegistryModule::GetRegistry().IsLoadingAssets())
    {
        YS_WARN_FUNC("Asset registry still loading assets...")
    }

    FAssetRegistryModule::GetRegistry().OnFilesLoaded().AddUObject(this, &UYarnSubsystem::OnAssetRegistryFilesLoaded);
    

    // FARFilter Filter;
    // Filter.PackageNames.Add(TEXT("NewFunctionLibrary"));
    // Filter.PackageNames.Add(TEXT("/Game/NewFunctionLibrary.NewFunctionLibrary"));
    // Filter.PackageNames.Add(TEXT("D:/dev/YarnSpinner/YSUEDemo4.27/Content/NewFunctionLibrary.uasset"));
    // Filter.PackageNames.Add(TEXT("/Game/NewFunctionLibrary"));
    Filter.PackageNames.Add(TEXT("/Game"));
    // TArray<FAssetData> AssetData;
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

    OnWorldInitializedActorsHandle = FWorldDelegates::OnWorldInitializedActors.AddLambda([this](const UWorld::FActorsInitializedParams& Params) //UWorld* World, const UWorld::InitializationValues IVS)
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
        
        FARFilter Filter;
        // Filter.PackageNames.Add(TEXT("NewFunctionLibrary"));
        // Filter.PackageNames.Add(TEXT("/Game/NewFunctionLibrary.NewFunctionLibrary"));
        // Filter.PackageNames.Add(TEXT("D:/dev/YarnSpinner/YSUEDemo4.27/Content/NewFunctionLibrary.uasset"));
        // Filter.PackageNames.Add(TEXT("/Game/NewFunctionLibrary"));
        // Filter.PackageNames.Add(TEXT("/Game"));
        // Filter.ClassNames.Add(AYarnFunctionLibrary::StaticClass()->GetFName());
        // Filter.ClassNames.Add("AYarnFunctionLibrary");
        Filter.ClassNames.Add("UBlueprint");
        TArray<FAssetData> AssetData;
        FAssetRegistryModule::GetRegistry().GetAssets(Filter, AssetData);
        if (AssetData.Num() == 0)
        {
            YS_WARN_FUNC("Asset search did not find any yarn function libraries")
            return;
        }

        for (auto Asset : AssetData)
        {
            auto YFL = Cast<AYarnFunctionLibrary>(Asset.GetAsset());
            if (YFL)
            {
                YS_LOG_FUNC("Found yarn function library: %s", *YFL->GetName())
                YarnFunctionLibrary = YFL;
                break;
            }
        }
    });

    // YS_LOG("function output device: %s", *Ar)

    OnLevelAddedToWorldHandle = FWorldDelegates::LevelAddedToWorld.AddUObject(this, &UYarnSubsystem::OnLevelAddedToWorld);
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


void UYarnSubsystem::OnAssetRegistryFilesLoaded()
{
    YS_LOG_FUNCSIG
    FARFilter Filter;
    // Filter.PackageNames.Add(TEXT("NewFunctionLibrary"));
    // Filter.PackageNames.Add(TEXT("/Game/NewFunctionLibrary.NewFunctionLibrary"));
    // Filter.PackageNames.Add(TEXT("D:/dev/YarnSpinner/YSUEDemo4.27/Content/NewFunctionLibrary.uasset"));
    // Filter.PackageNames.Add(TEXT("/Game/NewFunctionLibrary"));
    Filter.PackageNames.Add(TEXT("/Game"));
    Filter.ClassNames.Add(AYarnFunctionLibrary::StaticClass()->GetFName());
    TArray<FAssetData> AssetData;
    FAssetRegistryModule::GetRegistry().GetAssets(Filter, AssetData);
    if (AssetData.Num() == 0)
    {
        YS_WARN_FUNC("COULD NOT FIND ANY YARN FUNCTION LIBRARIES")
    }
    else
    {
        YS_LOG_FUNC("Found %d yarn function libraries", AssetData.Num())
    }
    
    // FARFilter Filter;
    // // Filter.PackageNames.Add(TEXT("NewFunctionLibrary"));
    // // Filter.PackageNames.Add(TEXT("/Game/NewFunctionLibrary.NewFunctionLibrary"));
    // // Filter.PackageNames.Add(TEXT("D:/dev/YarnSpinner/YSUEDemo4.27/Content/NewFunctionLibrary.uasset"));
    // // Filter.PackageNames.Add(TEXT("/Game/NewFunctionLibrary"));
    // Filter.PackageNames.Add(TEXT("/Game"));
    // Filter.ClassNames.Add(AYarnFunctionLibrary::StaticClass()->GetFName());
    // TArray<FAssetData> AssetData;
    // FAssetRegistryModule::GetRegistry().GetAssets(Filter, AssetData);
}


void UYarnSubsystem::OnLevelAddedToWorld(ULevel* Level, UWorld* World)
{
    YS_LOG_FUNCSIG

    OnAssetRegistryFilesLoaded();
}

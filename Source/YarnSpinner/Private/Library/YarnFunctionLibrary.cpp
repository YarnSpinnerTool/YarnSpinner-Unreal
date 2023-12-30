// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/YarnFunctionLibrary.h"

#include "Library/YarnLibraryRegistry.h"
#include "Misc/YSLogging.h"



// Sets default values
UYarnFunctionLibrary::UYarnFunctionLibrary()
{
    YS_LOG_FUNCSIG
    
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    // PrimaryActorTick.bCanEverTick = true;

    // Attempt to create subobjects from Blueprint-defined Blueprint Function Libraries

    // FARFilter Filter;
    // // Filter.PackageNames.Add(TEXT("/Game/NewFunctionLibrary"));
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
    //     // Asset.GetClass()->CreateDefaultSubobject<UBlueprintFunctionLibrary>()
    //     // Asset.GetClass()->
    //     // NewObject<UBlueprintFunctionLibrary>(Asset.GetClass());
    //     // CreateDefaultSubobject<UBlueprintFunctionLibrary>(Asset.AssetName, false);
    //     // Create
    //
    //     // NewObject
    //     
    // }
}


UYarnFunctionLibrary* UYarnFunctionLibrary::FromBlueprint(const UBlueprint* Blueprint)
{
    if (!Blueprint || !Blueprint->GeneratedClass || !Blueprint->GeneratedClass->GetDefaultObject())
        return nullptr;

    return Cast<UYarnFunctionLibrary>(Blueprint->GeneratedClass->GetDefaultObject());
}


TOptional<Yarn::FValue> UYarnFunctionLibrary::CallFunction(FName FunctionName, TArray<FYarnBlueprintParam> Args, TOptional<FYarnBlueprintParam> ReturnValue)
{
    TOptional<Yarn::FValue> Result;
    
    // Find the function
    UFunction* Function = FindFunction(FunctionName);

    if (!Function)
    {
        YS_WARN("Could not find function '%s'", *FunctionName.ToString())
        return Result;
    }

#if ENGINE_MAJOR_VERSION >= 5
    typedef FDoubleProperty NumericProperty;
#else
    typedef FFloatProperty NumericProperty;
#endif

    // Set up the parameters
    FStructOnScope FuncParams(Function);
    FBoolProperty* BoolParam;
    NumericProperty* NumericParam;
    FStrProperty* StringParam;
    
    for (auto Arg : Args)
    {
        // Set input properties
        switch (Arg.Value.GetType())
        {
        case Yarn::FValue::EValueType::Bool:
            BoolParam = CastField<FBoolProperty>(Function->FindPropertyByName(Arg.Name));
            if (!BoolParam)
            {
                YS_WARN_FUNC("Could not create function parameter '%s' for function %s from given values", *Arg.Name.ToString(), *FunctionName.ToString())
                return Result;
            }
            BoolParam->SetPropertyValue_InContainer(FuncParams.GetStructMemory(), Arg.Value.GetValue<bool>());
            break;
        case Yarn::FValue::EValueType::Number:
            NumericParam = CastField<NumericProperty>(Function->FindPropertyByName(Arg.Name));
            if (!NumericParam)
            {
                YS_WARN_FUNC("Could not create function parameter '%s' for function %s from given values", *Arg.Name.ToString(), *FunctionName.ToString())
                return Result;
            }
            NumericParam->SetPropertyValue_InContainer(FuncParams.GetStructMemory(), Arg.Value.GetValue<double>());
            break;
        case Yarn::FValue::EValueType::String:
            StringParam = CastField<FStrProperty>(Function->FindPropertyByName(Arg.Name));
            if (!StringParam)
            {
                YS_WARN_FUNC("Could not create function parameter '%s' for function %s from given values", *Arg.Name.ToString(), *FunctionName.ToString())
                return Result;
            }
            StringParam->SetPropertyValue_InContainer(FuncParams.GetStructMemory(), Arg.Value.GetValue<FString>());
            break;
        }
    }

    // Call the function
    ProcessEvent(Function, FuncParams.GetStructMemory());

    // Get the return value
    if (ReturnValue.IsSet())
    {
        switch (ReturnValue->Value.GetType())
        {
        case Yarn::FValue::EValueType::Bool:
            BoolParam = CastField<FBoolProperty>(Function->FindPropertyByName(ReturnValue->Name));
            if (!BoolParam)
            {
                YS_WARN_FUNC("Could not get return parameter '%s' for function '%s'", *ReturnValue->Name.ToString(), *FunctionName.ToString())
                return Result;
            }
            Result = Yarn::FValue(BoolParam->GetPropertyValue_InContainer(FuncParams.GetStructMemory()));
            break;
        case Yarn::FValue::EValueType::Number:
            NumericParam = CastField<NumericProperty>(Function->FindPropertyByName(ReturnValue->Name));
            if (!NumericParam)
            {
                YS_WARN_FUNC("Could not get return parameter '%s' for function '%s'", *ReturnValue->Name.ToString(), *FunctionName.ToString())
                return Result;
            }
            Result = Yarn::FValue(NumericParam->GetPropertyValue_InContainer(FuncParams.GetStructMemory()));
            break;
        case Yarn::FValue::EValueType::String:
            StringParam = CastField<FStrProperty>(Function->FindPropertyByName(ReturnValue->Name));
            if (!StringParam)
            {
                YS_WARN_FUNC("Could not get return parameter '%s' for function '%s'", *ReturnValue->Name.ToString(), *FunctionName.ToString())
                return Result;
            }
            Result = Yarn::FValue(TCHAR_TO_UTF8(*StringParam->GetPropertyValue_InContainer(FuncParams.GetStructMemory())));
            break;
        }
    }

    return Result;
}


// Called when the game starts or when spawned
// void UYarnFunctionLibrary::BeginPlay()
// {
//     YS_LOG_FUNCSIG
//     Super::BeginPlay();
//
//     /*
//     // Method #1 -- Does nothing.
//     auto Ar = FOutputDeviceNull();
//     CallFunctionByNameWithArguments(TEXT("MyQuickActorFunction 3.14"), Ar, nullptr, true);
//
//     auto Function = FindFunction("MyQuickActorFunction");
//     if (Function)
//     {
//         YS_LOG_FUNC("FOUND IT")
//     }
//     else
//     {
//         YS_LOG_FUNC("NOT FOUND")
//     }
//
//
//     // Method #2 -- Works
//     struct FLocalParameters
//     {
//         float InParam = 12;
//     };
//     FLocalParameters Parameters;
//     ProcessEvent(Function, &Parameters);
//     
//
//     // Method #3 -- Works, gives return value
//     float InFloat = 76.0;
//     FStructOnScope FuncParam(Function);
//
//     // Set input properties
//     FFloatProperty* InParam = CastField<FFloatProperty>(Function->FindPropertyByName(TEXT("InParam")));
//     if (!InParam)
//     {
//         YS_WARN_FUNC("Could not create InProp")
//         return;
//     }
//     InParam->SetPropertyValue_InContainer(FuncParam.GetStructMemory(), InFloat);
//
//     ProcessEvent(Function, FuncParam.GetStructMemory());
//
//     FBoolProperty* OutParam = CastField<FBoolProperty>(Function->FindPropertyByName(TEXT("OutParam")));
//     if (!OutParam)
//     {
//         YS_WARN_FUNC("Could not create OutProp")
//         return;
//     }
//     bool OutBool = OutParam->GetPropertyValue_InContainer(FuncParam.GetStructMemory());
//     if (OutBool == true)
//     {
//         YS_LOG_FUNC("REUTRNEDD TRUEEEEE")
//     }
//     else
//     {
//         YS_LOG_FUNC("RETURENEEDD FAALSLSLKESEEEE")
//     }
//     */
// }


// Called every frame
// void UYarnFunctionLibrary::Tick(float DeltaTime)
// {
//     Super::Tick(DeltaTime);
//
// }


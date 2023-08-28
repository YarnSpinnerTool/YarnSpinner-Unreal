// Fill out your copyright notice in the Description page of Project Settings.


#include "YarnFunctionLibrary.h"

#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/WidgetComponent.h"
#include "Misc/OutputDeviceNull.h"
#include "Misc/YSLogging.h"



// Sets default values
AYarnFunctionLibrary::AYarnFunctionLibrary()
{
    YS_LOG_FUNCSIG
    
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

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


TOptional<Yarn::Value> AYarnFunctionLibrary::CallFunction(FName FunctionName, TArray<FYarnBlueprintArg> Args, TOptional<FYarnBlueprintArg> ReturnValue)
{
    TOptional<Yarn::Value> Result;
    
    // Find the function
    UFunction* Function = FindFunction(FunctionName);

    if (!Function)
    {
        YS_WARN("Could not find function %s", *FunctionName.ToString())
        return Result;
    }

    // Set up the parameters
    FStructOnScope FuncParams(Function);
    FBoolProperty* BoolParam;
    FFloatProperty* FloatParam;
    FStrProperty* StringParam;
    
    for (auto Arg : Args)
    {
        // Set input properties
        switch (Arg.Value.GetType())
        {
        case Yarn::Value::ValueType::BOOL:
            BoolParam = CastField<FBoolProperty>(Function->FindPropertyByName(Arg.Name));
            if (!BoolParam)
            {
                YS_WARN_FUNC("Could not create function parameter %s for function %s from given values", *Arg.Name.ToString(), *FunctionName.ToString())
                return Result;
            }
            BoolParam->SetPropertyValue_InContainer(FuncParams.GetStructMemory(), Arg.Value.GetBooleanValue());
            break;
        case Yarn::Value::ValueType::NUMBER:
            FloatParam = CastField<FFloatProperty>(Function->FindPropertyByName(Arg.Name));
            if (!FloatParam)
            {
                YS_WARN_FUNC("Could not create function parameter %s for function %s from given values", *Arg.Name.ToString(), *FunctionName.ToString())
                return Result;
            }
            FloatParam->SetPropertyValue_InContainer(FuncParams.GetStructMemory(), Arg.Value.GetNumberValue());
            break;
        case Yarn::Value::ValueType::STRING:
            StringParam = CastField<FStrProperty>(Function->FindPropertyByName(Arg.Name));
            if (!StringParam)
            {
                YS_WARN_FUNC("Could not create function parameter %s for function %s from given values", *Arg.Name.ToString(), *FunctionName.ToString())
                return Result;
            }
            StringParam->SetPropertyValue_InContainer(FuncParams.GetStructMemory(), FString(Arg.Value.GetStringValue().c_str()));
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
        case Yarn::Value::ValueType::BOOL:
            BoolParam = CastField<FBoolProperty>(Function->FindPropertyByName(ReturnValue->Name));
            if (!BoolParam)
            {
                YS_WARN_FUNC("Could not get return parameter %s for function %s", *ReturnValue->Name.ToString(), *FunctionName.ToString())
                return Result;
            }
            Result = Yarn::Value(BoolParam->GetPropertyValue_InContainer(FuncParams.GetStructMemory()));
            break;
        case Yarn::Value::ValueType::NUMBER:
            FloatParam = CastField<FFloatProperty>(Function->FindPropertyByName(ReturnValue->Name));
            if (!FloatParam)
            {
                YS_WARN_FUNC("Could not get return parameter %s for function %s", *ReturnValue->Name.ToString(), *FunctionName.ToString())
                return Result;
            }
            Result = Yarn::Value(FloatParam->GetPropertyValue_InContainer(FuncParams.GetStructMemory()));
            break;
        case Yarn::Value::ValueType::STRING:
            StringParam = CastField<FStrProperty>(Function->FindPropertyByName(ReturnValue->Name));
            if (!StringParam)
            {
                YS_WARN_FUNC("Could not get return parameter %s for function %s", *ReturnValue->Name.ToString(), *FunctionName.ToString())
                return Result;
            }
            Result = Yarn::Value(TCHAR_TO_UTF8(*StringParam->GetPropertyValue_InContainer(FuncParams.GetStructMemory())));
            break;
        }
    }

    return Result;
}


// Called when the game starts or when spawned
void AYarnFunctionLibrary::BeginPlay()
{
    YS_LOG_FUNCSIG
    Super::BeginPlay();

    /*
    // Method #1 -- Does nothing.
    auto Ar = FOutputDeviceNull();
    CallFunctionByNameWithArguments(TEXT("MyQuickActorFunction 3.14"), Ar, nullptr, true);

    auto Function = FindFunction("MyQuickActorFunction");
    if (Function)
    {
        YS_LOG_FUNC("FOUND IT")
    }
    else
    {
        YS_LOG_FUNC("NOT FOUND")
    }


    // Method #2 -- Works
    struct FLocalParameters
    {
        float InParam = 12;
    };
    FLocalParameters Parameters;
    ProcessEvent(Function, &Parameters);
    

    // Method #3 -- Works, gives return value
    float InFloat = 76.0;
    FStructOnScope FuncParam(Function);

    // Set input properties
    FFloatProperty* InParam = CastField<FFloatProperty>(Function->FindPropertyByName(TEXT("InParam")));
    if (!InParam)
    {
        YS_WARN_FUNC("Could not create InProp")
        return;
    }
    InParam->SetPropertyValue_InContainer(FuncParam.GetStructMemory(), InFloat);

    ProcessEvent(Function, FuncParam.GetStructMemory());

    FBoolProperty* OutParam = CastField<FBoolProperty>(Function->FindPropertyByName(TEXT("OutParam")));
    if (!OutParam)
    {
        YS_WARN_FUNC("Could not create OutProp")
        return;
    }
    bool OutBool = OutParam->GetPropertyValue_InContainer(FuncParam.GetStructMemory());
    if (OutBool == true)
    {
        YS_LOG_FUNC("REUTRNEDD TRUEEEEE")
    }
    else
    {
        YS_LOG_FUNC("RETURENEEDD FAALSLSLKESEEEE")
    }
    */
}


// Called every frame
void AYarnFunctionLibrary::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}


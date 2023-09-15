// Fill out your copyright notice in the Description page of Project Settings.


#include "YarnFunctionRegistry.h"

#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "YarnFunctionLibrary.h"
#include "Misc/DefaultValueHelper.h"
#include "Misc/YarnAssetHelpers.h"
#include "Misc/YSLogging.h"


UYarnFunctionRegistry::UYarnFunctionRegistry()
{
    YS_LOG_FUNCSIG
    const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    // Set up asset registry delegates
    {
        OnAssetRegistryFilesLoadedHandle = AssetRegistry.OnFilesLoaded().AddUObject(this, &UYarnFunctionRegistry::OnAssetRegistryFilesLoaded);
        OnAssetAddedHandle = AssetRegistry.OnAssetAdded().AddUObject(this, &UYarnFunctionRegistry::OnAssetAdded);
        OnAssetRemovedHandle = AssetRegistry.OnAssetRemoved().AddUObject(this, &UYarnFunctionRegistry::OnAssetRemoved);
        OnAssetUpdatedHandle = AssetRegistry.OnAssetUpdated().AddUObject(this, &UYarnFunctionRegistry::OnAssetUpdated);
        OnAssetRenamedHandle = AssetRegistry.OnAssetRenamed().AddUObject(this, &UYarnFunctionRegistry::OnAssetRenamed);
        FWorldDelegates::OnStartGameInstance.AddUObject(this, &UYarnFunctionRegistry::OnStartGameInstance);
    }
}


void UYarnFunctionRegistry::BeginDestroy()
{
    YS_LOG_FUNCSIG
    UObject::BeginDestroy();

    IAssetRegistry& AssetRegistry = FAssetRegistryModule::GetRegistry();
    AssetRegistry.OnFilesLoaded().Remove(OnAssetRegistryFilesLoadedHandle);
    AssetRegistry.OnAssetAdded().Remove(OnAssetAddedHandle);
    AssetRegistry.OnAssetRemoved().Remove(OnAssetRemovedHandle);
    AssetRegistry.OnAssetRenamed().Remove(OnAssetRenamedHandle);
}


UBlueprint* UYarnFunctionRegistry::GetYarnFunctionLibraryBlueprint(const FAssetData& AssetData) const
{
    UBlueprint* BP = Cast<UBlueprint>(AssetData.GetAsset());

    if (!BP)
    {
        // YS_LOG_FUNC("Could not cast asset to UBlueprint")
        return nullptr;
    }
    if (!BP->ParentClass->IsChildOf<UYarnFunctionLibrary>())
    {
        // YS_LOG_FUNC("Blueprint is not a child of UYarnFunctionLibrary")
        return nullptr;
    }
    if (!BP->GeneratedClass)
    {
        // YS_LOG_FUNC("Blueprint has no generated class")
        return nullptr;
    }

    return BP;
}


void UYarnFunctionRegistry::FindFunctions()
{
    YS_LOG_FUNCSIG
    YS_LOG("Project content dir: %s", *FPaths::ProjectContentDir());

    TArray<FAssetData> ExistingAssets = FYarnAssetHelpers::FindAssetsInRegistryByPackagePath<UBlueprint>(FPaths::GetPath("/Game/"));

    for (auto Asset : ExistingAssets)
    {
        YS_LOG_FUNC("Found asset: %s (%s) %s", *Asset.GetFullName(), *Asset.GetPackage()->GetName(), *Asset.GetAsset()->GetPathName())
        UBlueprint* BP = GetYarnFunctionLibraryBlueprint(Asset);
        ImportFunctions(BP);
    }
}


void UYarnFunctionRegistry::ImportFunctions(UBlueprint* YarnFunctionLibrary)
{
    YS_LOG_FUNCSIG

    // Simple example function that takes a bool, float and string and returns a string
    // (the original string if the bool is true, otherwise the float as a string):
    //
    // Found asset: Blueprint /Game/AnotherYarnFuncLib.AnotherYarnFuncLib (/Game/AnotherYarnFuncLib) /Game/AnotherYarnFuncLib.AnotherYarnFuncLib
    // Function graph: DoStuff
    // Node: K2Node_FunctionEntry_0 (Class: K2Node_FunctionEntry, Title: Do Stuff)
    // Node is a function entry node with 4 pins
    // Pin: then (Direction: 1, PinType: exec)
    // Pin: FirstInputParam (Direction: 1, PinType: bool)
    // Pin: SecondInputParam (Direction: 1, PinType: float)
    // Pin: ThirdInputParam (Direction: 1, PinType: string)
    // Node: K2Node_IfThenElse_0 (Class: K2Node_IfThenElse, Title: Branch)
    // Pin: execute (Direction: 0, PinType: exec)
    // Pin: Condition (Direction: 0, PinType: bool)
    // Pin: then (Direction: 1, PinType: exec)
    // Pin: else (Direction: 1, PinType: exec)
    // Node: K2Node_FunctionResult_0 (Class: K2Node_FunctionResult, Title: Return Node)
    // Node is a function result node with 2 pins
    // Pin: execute (Direction: 0, PinType: exec)
    // Pin: OutputParam (Direction: 0, PinType: string)
    // Node: K2Node_CallFunction_0 (Class: K2Node_CallFunction, Title: ToString (float))
    // Pin: self (Direction: 0, PinType: object)
    // Pin: InFloat (Direction: 0, PinType: float)
    // Pin: ReturnValue (Direction: 1, PinType: string)
    // Node: K2Node_FunctionResult_1 (Class: K2Node_FunctionResult, Title: Return Node)
    // Node is a function result node with 2 pins
    // Pin: execute (Direction: 0, PinType: exec)
    // Pin: OutputParam (Direction: 0, PinType: string)

    if (!YarnFunctionLibrary)
    {
        YS_WARN_FUNC("null Blueprint library passed in")
        return;
    }

    // Store a reference to the Blueprint library
    Libraries.Add(YarnFunctionLibrary);
    LibFunctions.Add(YarnFunctionLibrary, {});

    for (UEdGraph* Func : YarnFunctionLibrary->FunctionGraphs)
    {
        YS_LOG("Function graph: %s", *Func->GetName())

        FYarnBPLibFunction FuncDetails;
        FuncDetails.Library = YarnFunctionLibrary;
        FuncDetails.Name = Func->GetFName();

        // Find function entry and return nodes, ensure they have a valid number of pins, etc
        for (auto Node : Func->Nodes)
        {
            YS_LOG("Node: %s (Class: %s, Title: %s)", *Node->GetName(), *Node->GetClass()->GetName(), *Node->GetNodeTitle(ENodeTitleType::ListView).ToString())
            if (Node->IsA(UK2Node_FunctionEntry::StaticClass()))
            {
                YS_LOG("Node is a function entry node with %d pins", Node->Pins.Num())
                for (auto Pin : Node->Pins)
                {
                    auto& Category = Pin->PinType.PinCategory;
                    if (Category == TEXT("bool"))
                    {
                        FYarnBPFuncParam Param;
                        Param.Name = Pin->PinName;
                        bool Value = Pin->GetDefaultAsString() == "true";
                        Param.Value = Yarn::Value(Value);
                        FuncDetails.InParams.Add(Param);
                    }
                    else if (Category == TEXT("float"))
                    {
                        FYarnBPFuncParam Param;
                        Param.Name = Pin->PinName;
                        float Value = 0;
                        FDefaultValueHelper::ParseFloat(Pin->GetDefaultAsString(), Value);
                        Param.Value = Yarn::Value(Value);
                        FuncDetails.InParams.Add(Param);
                    }
                    else if (Category == TEXT("string"))
                    {
                        FYarnBPFuncParam Param;
                        Param.Name = Pin->PinName;
                        Param.Value = Yarn::Value(TCHAR_TO_UTF8(*Pin->GetDefaultAsString()));
                        FuncDetails.InParams.Add(Param);
                    }
                    else if (Category != TEXT("exec"))
                    {
                        YS_WARN("Invalid Yarn function entry pin type: %s.  Must be bool, float or string.", *Pin->PinType.PinCategory.ToString())
                    }
                }
            }
            else if (Node->IsA(UK2Node_FunctionResult::StaticClass()))
            {
                YS_LOG("Node is a function result node with %d pins", Node->Pins.Num())
                for (auto Pin : Node->Pins)
                {
                    auto& Category = Pin->PinType.PinCategory;
                    if (Category == TEXT("bool"))
                    {
                        if (FuncDetails.OutParam.IsSet() && (FuncDetails.OutParam->Value.GetType() != Yarn::Value::ValueType::BOOL || FuncDetails.OutParam->Name != Pin->PinName))
                        {
                            YS_WARN("Function %s has multiple return values or types.  Only one is supported.", *FuncDetails.Name.ToString())
                        }
                        FYarnBPFuncParam Param;
                        Param.Name = Pin->PinName;
                        bool Value = Pin->GetDefaultAsString() == "true";
                        Param.Value = Yarn::Value(Value);
                        FuncDetails.OutParam = Param;
                    }
                    else if (Category == TEXT("float"))
                    {
                        if (FuncDetails.OutParam.IsSet() && (FuncDetails.OutParam->Value.GetType() != Yarn::Value::ValueType::NUMBER || FuncDetails.OutParam->Name != Pin->PinName))
                        {
                            YS_WARN("Function %s has multiple return values or types.  Only one is supported.", *FuncDetails.Name.ToString())
                        }
                        FYarnBPFuncParam Param;
                        Param.Name = Pin->PinName;
                        float Value = 0;
                        FDefaultValueHelper::ParseFloat(Pin->GetDefaultAsString(), Value);
                        Param.Value = Yarn::Value(Value);
                        FuncDetails.OutParam = Param;
                    }
                    else if (Category == TEXT("string"))
                    {
                        if (FuncDetails.OutParam.IsSet() && (FuncDetails.OutParam->Value.GetType() != Yarn::Value::ValueType::STRING || FuncDetails.OutParam->Name != Pin->PinName))
                        {
                            YS_WARN("Function %s has multiple return values or types.  Only one is supported.", *FuncDetails.Name.ToString())
                        }
                        FYarnBPFuncParam Param;
                        Param.Name = Pin->PinName;
                        Param.Value = Yarn::Value(TCHAR_TO_UTF8(*Pin->GetDefaultAsString()));
                        FuncDetails.OutParam = Param;
                    }
                    else if (Category != TEXT("exec"))
                    {
                        YS_WARN("Invalid Yarn function result pin type: %s.  Must be bool, float or string.", *Pin->PinType.PinCategory.ToString())
                    }
                }
            }
            else if (Node->IsA(UK2Node_FunctionTerminator::StaticClass()))
            {
                YS_LOG("Node is a function terminator node with %d pins", Node->Pins.Num())
            }
            for (auto Pin : Node->Pins)
            {
                YS_LOG("Pin: %s (Direction: %d, PinType: %s, DefaultValue: %s)", *Pin->GetName(), (int)Pin->Direction, *Pin->PinType.PinCategory.ToString(), *Pin->GetDefaultAsString())
            }
        }

        LibFunctions[YarnFunctionLibrary].Add(FuncDetails.Name);
        AllFunctions.FindOrAdd(FuncDetails.Name).Add(FuncDetails);
    }
}


void UYarnFunctionRegistry::UpdateFunctions(UBlueprint* YarnFunctionLibrary)
{
    YS_LOG_FUNCSIG
    RemoveFunctions(YarnFunctionLibrary);
    ImportFunctions(YarnFunctionLibrary);
}


void UYarnFunctionRegistry::RemoveFunctions(UBlueprint* YarnFunctionLibrary)
{
    YS_LOG_FUNCSIG
    if (Libraries.Contains(YarnFunctionLibrary))
    {
        if (LibFunctions.Contains(YarnFunctionLibrary))
        {
            for (auto FuncName : LibFunctions[YarnFunctionLibrary])
            {
                if (AllFunctions.Contains(FuncName))
                {
                    AllFunctions[FuncName].RemoveAll([YarnFunctionLibrary](const FYarnBPLibFunction& Function) { return Function.Library == YarnFunctionLibrary; });
                }
                AllFunctions.Remove(FuncName);
            }
            LibFunctions.Remove(YarnFunctionLibrary);
        }
        Libraries.Remove(YarnFunctionLibrary);
    }
}


void UYarnFunctionRegistry::OnAssetRegistryFilesLoaded()
{
    YS_LOG_FUNCSIG
    bRegistryFilesLoaded = true;
    FindFunctions();
}


void UYarnFunctionRegistry::OnAssetAdded(const FAssetData& AssetData)
{
    // Ignore this until the registry has finished loading the first time.
    if (!bRegistryFilesLoaded)
        return;
    YS_LOG_FUNCSIG

    UBlueprint* BP = GetYarnFunctionLibraryBlueprint(AssetData);
    if (BP)
    {
        ImportFunctions(BP);
    }
}


void UYarnFunctionRegistry::OnAssetRemoved(const FAssetData& AssetData)
{
    YS_LOG_FUNCSIG
    UBlueprint* BP = GetYarnFunctionLibraryBlueprint(AssetData);
    if (BP)
    {
        RemoveFunctions(BP);
    }
}


void UYarnFunctionRegistry::OnAssetUpdated(const FAssetData& AssetData)
{
    YS_LOG_FUNCSIG
    UBlueprint* BP = GetYarnFunctionLibraryBlueprint(AssetData);
    if (BP)
    {
        UpdateFunctions(BP);
    }
}


void UYarnFunctionRegistry::OnAssetRenamed(const FAssetData& AssetData, const FString& String)
{
    YS_LOG_FUNCSIG
    UBlueprint* BP = GetYarnFunctionLibraryBlueprint(AssetData);
    if (BP)
    {
        UpdateFunctions(BP);
    }
}


void UYarnFunctionRegistry::OnStartGameInstance(UGameInstance* GameInstance)
{
    YS_LOG_FUNCSIG
    FindFunctions();
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/YarnLibraryRegistry.h"

#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "Library/YarnCommandLibrary.h"
#include "Library/YarnFunctionLibrary.h"
#include "Misc/DefaultValueHelper.h"
#include "Misc/YarnAssetHelpers.h"
#include "Misc/YSLogging.h"


UYarnLibraryRegistry::UYarnLibraryRegistry()
{
    YS_LOG_FUNCSIG
    const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    // Set up asset registry delegates
    {
        OnAssetRegistryFilesLoadedHandle = AssetRegistry.OnFilesLoaded().AddUObject(this, &UYarnLibraryRegistry::OnAssetRegistryFilesLoaded);
        OnAssetAddedHandle = AssetRegistry.OnAssetAdded().AddUObject(this, &UYarnLibraryRegistry::OnAssetAdded);
        OnAssetRemovedHandle = AssetRegistry.OnAssetRemoved().AddUObject(this, &UYarnLibraryRegistry::OnAssetRemoved);
        OnAssetUpdatedHandle = AssetRegistry.OnAssetUpdated().AddUObject(this, &UYarnLibraryRegistry::OnAssetUpdated);
        OnAssetRenamedHandle = AssetRegistry.OnAssetRenamed().AddUObject(this, &UYarnLibraryRegistry::OnAssetRenamed);
        FWorldDelegates::OnStartGameInstance.AddUObject(this, &UYarnLibraryRegistry::OnStartGameInstance);
    }

    LoadStdFunctions();
}


void UYarnLibraryRegistry::BeginDestroy()
{
    UObject::BeginDestroy();

    IAssetRegistry& AssetRegistry = FAssetRegistryModule::GetRegistry();
    AssetRegistry.OnFilesLoaded().Remove(OnAssetRegistryFilesLoadedHandle);
    AssetRegistry.OnAssetAdded().Remove(OnAssetAddedHandle);
    AssetRegistry.OnAssetRemoved().Remove(OnAssetRemovedHandle);
    AssetRegistry.OnAssetRenamed().Remove(OnAssetRenamedHandle);
}


bool UYarnLibraryRegistry::HasFunction(const FName& Name) const
{
    if (StdFunctions.Contains(Name))
        return true;
    
    if (!AllFunctions.Contains(Name))
    {
        FString S = "Could not find function '" + Name.ToString() + "'.  Known functions: ";
        for (auto Func : AllFunctions)
        {
            S += "'" + Func.Key.ToString() + "', ";
        }
        YS_WARN("%s", *S)
    }
    
    return AllFunctions.Contains(Name);
}


int32 UYarnLibraryRegistry::GetExpectedFunctionParamCount(const FName& Name) const
{
    if (StdFunctions.Contains(Name))
        return StdFunctions[Name].ExpectedParamCount;
    
    if (!AllFunctions.Contains(Name))
    {
        YS_WARN("Could not find function '%s' in registry.", *Name.ToString())
        return 0;
    }
    return AllFunctions[Name].InParams.Num();
}


Yarn::Value UYarnLibraryRegistry::CallFunction(const FName& Name, TArray<Yarn::Value> Parameters) const
{
    if (StdFunctions.Contains(Name))
    {
        return StdFunctions[Name].Function(Parameters);
    }
    
    if (!AllFunctions.Contains(Name))
    {
        YS_WARN("Attempted to call non-existent function '%s'", *Name.ToString())
        return Yarn::Value();
    }

    auto FuncDetail = AllFunctions[Name];

    if (FuncDetail.InParams.Num() != Parameters.Num())
    {
        YS_WARN("Attempted to call function '%s' with incorrect number of arguments (expected %d).", *Name.ToString(), FuncDetail.InParams.Num())
        return Yarn::Value();
    }

    auto Lib = UYarnFunctionLibrary::FromBlueprint(FuncDetail.Library);
    
    if (!Lib)
    {
        YS_WARN("Couldn't create library for Blueprint containing function '%s'", *Name.ToString())
        return Yarn::Value();
    }

    TArray<FYarnBlueprintFuncParam> InParams;
    for (auto Param : FuncDetail.InParams)
    {
        FYarnBlueprintFuncParam InParam = Param;
        InParam.Value = Parameters[InParams.Num()];
        InParams.Add(InParam);
    }

    TOptional<FYarnBlueprintFuncParam> OutParam = FuncDetail.OutParam;

    auto Result = Lib->CallFunction(Name, InParams, OutParam);
    if (Result.IsSet())
    {
        return Result.GetValue();
    }
    YS_WARN("Function '%s' returned an invalid value.", *Name.ToString())
    return Yarn::Value();
}


UBlueprint* UYarnLibraryRegistry::GetYarnFunctionLibraryBlueprint(const FAssetData& AssetData)
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


UBlueprint* UYarnLibraryRegistry::GetYarnCommandLibraryBlueprint(const FAssetData& AssetData)
{
    UBlueprint* BP = Cast<UBlueprint>(AssetData.GetAsset());

    if (!BP)
    {
        // YS_LOG_FUNC("Could not cast asset to UBlueprint")
        return nullptr;
    }
    if (!BP->ParentClass->IsChildOf<UYarnCommandLibrary>())
    {
        // YS_LOG_FUNC("Blueprint is not a child of UYarnCommandLibrary")
        return nullptr;
    }
    if (!BP->GeneratedClass)
    {
        // YS_LOG_FUNC("Blueprint has no generated class")
        return nullptr;
    }

    return BP;
}


void UYarnLibraryRegistry::FindFunctionsAndCommands()
{
    YS_LOG_FUNCSIG
    YS_LOG("Project content dir: %s", *FPaths::ProjectContentDir());

    TArray<FAssetData> ExistingAssets = FYarnAssetHelpers::FindAssetsInRegistryByPackagePath<UBlueprint>(FPaths::GetPath("/Game/"));
    // TArray<FAssetData> ExistingAssets = FYarnAssetHelpers::FindAssetsInRegistry<UBlueprint>();

    for (auto Asset : ExistingAssets)
    {
        YS_LOG_FUNC("Found Blueprint asset: %s (%s) %s", *Asset.GetFullName(), *Asset.GetPackage()->GetName(), *Asset.GetAsset()->GetPathName())
        if (UBlueprint* BP = GetYarnFunctionLibraryBlueprint(Asset))
        {
            ImportFunctions(BP);
        }
        if (UBlueprint* BP = GetYarnCommandLibraryBlueprint(Asset))
        {
            ImportCommands(BP);
        }
    }
}


void UYarnLibraryRegistry::ExtractFunctionDataFromBlueprintGraph(UBlueprint* YarnFunctionLibrary, UEdGraph* Func, FYarnBlueprintLibFunction& FuncDetails, FYarnBlueprintLibFunctionMeta& FuncMeta)
{
    YS_LOG("Function graph: %s", *Func->GetName())
    FuncDetails.Library = YarnFunctionLibrary;
    FuncDetails.Name = Func->GetFName();
    
    // Here is the graph of a simple example function that takes a bool, float and string and returns a string (the original string if the bool is true, otherwise the float as a string):
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

    // Find function entry and return nodes, ensure they have a valid number of pins, etc
    for (auto Node : Func->Nodes)
    {
        YS_LOG("Node: %s (Class: %s, Title: %s)", *Node->GetName(), *Node->GetClass()->GetName(), *Node->GetNodeTitle(ENodeTitleType::ListView).ToString())
        if (Node->IsA(UK2Node_FunctionEntry::StaticClass()))
        {
            auto EntryNode = CastChecked<UK2Node_FunctionEntry>(Node);
            if (EntryNode->GetFunctionFlags() & FUNC_BlueprintPure)
            {
                FuncMeta.bIsPure = true;
                // YS_LOG("BLUEPRINT PURE")
            }
            if (EntryNode->GetFunctionFlags() & FUNC_Public)
            {
                FuncMeta.bIsPublic = true;
                // YS_LOG("PUBLIC FUNCTION")
            }
            if (EntryNode->GetFunctionFlags() & FUNC_Const)
            {
                FuncMeta.bIsConst = true;
                // YS_LOG("CONST FUNCTION")
            }
                    
            YS_LOG("Node is a function entry node with %d pins", Node->Pins.Num())
            for (auto Pin : Node->Pins)
            {
                auto& Category = Pin->PinType.PinCategory;
                if (Category == TEXT("bool"))
                {
                    FYarnBlueprintFuncParam Param;
                    Param.Name = Pin->PinName;
                    bool Value = Pin->GetDefaultAsString() == "true";
                    Param.Value = Yarn::Value(Value);
                    FuncDetails.InParams.Add(Param);
                }
                else if (Category == TEXT("float"))
                {
                    FYarnBlueprintFuncParam Param;
                    Param.Name = Pin->PinName;
                    float Value = 0;
                    FDefaultValueHelper::ParseFloat(Pin->GetDefaultAsString(), Value);
                    Param.Value = Yarn::Value(Value);
                    FuncDetails.InParams.Add(Param);
                }
                else if (Category == TEXT("string"))
                {
                    FYarnBlueprintFuncParam Param;
                    Param.Name = Pin->PinName;
                    Param.Value = Yarn::Value(TCHAR_TO_UTF8(*Pin->GetDefaultAsString()));
                    FuncDetails.InParams.Add(Param);
                }
                else if (Category != TEXT("exec"))
                {
                    // YS_WARN("Invalid Yarn function input parameter type: %s.  Must be bool, float or string.", *Pin->PinType.PinCategory.ToString())
                    FuncMeta.InvalidParams.Add(Pin->PinName.ToString());
                }
            }
            YS_LOG("Added %d parameters to function details", FuncDetails.InParams.Num())
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
                        // YS_WARN("Function %s has multiple return values or types.  Yarn functions may only have one return value.", *FuncDetails.Name.ToString())
                        FuncMeta.bHasMultipleOutParams = true;
                    }
                    FYarnBlueprintFuncParam Param;
                    Param.Name = Pin->PinName;
                    bool Value = Pin->GetDefaultAsString() == "true";
                    Param.Value = Yarn::Value(Value);
                    FuncDetails.OutParam = Param;
                }
                else if (Category == TEXT("float"))
                {
                    if (FuncDetails.OutParam.IsSet() && (FuncDetails.OutParam->Value.GetType() != Yarn::Value::ValueType::NUMBER || FuncDetails.OutParam->Name != Pin->PinName))
                    {
                        // YS_WARN("Function %s has multiple return values or types.  Only one is supported.", *FuncDetails.Name.ToString())
                        FuncMeta.bHasMultipleOutParams = true;
                    }
                    FYarnBlueprintFuncParam Param;
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
                        // YS_WARN("Function %s has multiple return values or types.  Only one is supported.", *FuncDetails.Name.ToString())
                        FuncMeta.bHasMultipleOutParams = true;
                    }
                    FYarnBlueprintFuncParam Param;
                    Param.Name = Pin->PinName;
                    Param.Value = Yarn::Value(TCHAR_TO_UTF8(*Pin->GetDefaultAsString()));
                    FuncDetails.OutParam = Param;
                }
                else if (Category != TEXT("exec"))
                {
                    // YS_WARN("Invalid Yarn function result pin type: %s.  Must be bool, float or string.", *Pin->PinType.PinCategory.ToString())
                    FuncMeta.InvalidParams.Add(Pin->PinName.ToString());
                }
            }
        }
        else if (Node->IsA(UK2Node_FunctionTerminator::StaticClass()))
        {
            // YS_LOG("Node is a function terminator node with %d pins", Node->Pins.Num())
        }
        for (auto Pin : Node->Pins)
        {
            // YS_LOG("Pin: %s (Direction: %d, PinType: %s, DefaultValue: %s)", *Pin->GetName(), (int)Pin->Direction, *Pin->PinType.PinCategory.ToString(), *Pin->GetDefaultAsString())
        }
    }
}


void UYarnLibraryRegistry::ImportFunctions(UBlueprint* YarnFunctionLibrary)
{
    if (!YarnFunctionLibrary)
    {
        YS_WARN_FUNC("null Blueprint library passed in")
        return;
    }

    // Store a reference to the Blueprint library
    FunctionLibraries.Add(YarnFunctionLibrary);
    LibFunctions.Add(YarnFunctionLibrary, {});

    for (UEdGraph* Func : YarnFunctionLibrary->FunctionGraphs)
    {
        FYarnBlueprintLibFunction FuncDetails;
        FYarnBlueprintLibFunctionMeta FuncMeta;
        
        ExtractFunctionDataFromBlueprintGraph(YarnFunctionLibrary, Func, FuncDetails, FuncMeta);

        // Ignore private functions
        if (FuncMeta.bIsPublic)
        {
            // Test for valid function
            bool bIsValid = true;
            if (FuncMeta.bHasMultipleOutParams || !FuncDetails.OutParam.IsSet())
            {
                bIsValid = false;
                YS_WARN("Function '%s' must have exactly one return parameter.", *FuncDetails.Name.ToString())
            }
            if (FuncMeta.InvalidParams.Num() > 0)
            {
                bIsValid = false;
                YS_WARN("Function %s has invalid parameter types. Yarn functions only support boolean, float and string.", *FuncDetails.Name.ToString())
            }
            // Check name is unique
            if (AllFunctions.Contains(FuncDetails.Name) && AllFunctions[FuncDetails.Name].Library != FuncDetails.Library)
            {
                bIsValid = false;
                YS_WARN("Function %s already exists in another Blueprint.  Yarn function names must be unique.", *FuncDetails.Name.ToString())
            }
            // Check name is valid
            const FRegexPattern Pattern{TEXT("^[\\w_][\\w\\d_]*$")};
            FRegexMatcher FunctionNameMatcher(Pattern, FuncDetails.Name.ToString());
            if (!FunctionNameMatcher.FindNext())
            {
                bIsValid = false;
                YS_WARN("Function %s has an invalid name.  Yarn function names must be valid C++ function names.", *FuncDetails.Name.ToString())
            }
            
            // TODO: check function has no async nodes
            
            // Add to function sets
            if (bIsValid)
            {
                LibFunctions.FindOrAdd(YarnFunctionLibrary).Add(FuncDetails.Name);
                AllFunctions.Add(FuncDetails.Name, FuncDetails);
            }
        }
    }
}


void UYarnLibraryRegistry::ImportCommands(UBlueprint* YarnCommandLibrary)
{
    if (!YarnCommandLibrary)
    {
        // YS_WARN_FUNC("null Blueprint library passed in")
        return;
    }

    // Store a reference to the Blueprint library
    CommandLibraries.Add(YarnCommandLibrary);
    LibCommands.FindOrAdd(YarnCommandLibrary);

    for (UEdGraph* Func : YarnCommandLibrary->FunctionGraphs)
    {
        FYarnBlueprintLibFunction FuncDetails;
        FYarnBlueprintLibFunctionMeta FuncMeta;
        
        ExtractFunctionDataFromBlueprintGraph(YarnCommandLibrary, Func, FuncDetails, FuncMeta);

        // Ignore private functions
        if (FuncMeta.bIsPublic)
        {
            // Test for valid function
            bool bIsValid = true;
            if (FuncDetails.OutParam.IsSet())
            {
                bIsValid = false;
                YS_WARN("Function %s has a return value.  Yarn commands must not return values.", *FuncDetails.Name.ToString())
            }
            if (FuncMeta.InvalidParams.Num() > 0)
            {
                bIsValid = false;
                YS_WARN("Function %s has invalid parameter types. Yarn commands only support boolean, float and string.", *FuncDetails.Name.ToString())
            }
            // Check name is unique
            if (AllFunctions.Contains(FuncDetails.Name) && AllFunctions[FuncDetails.Name].Library != FuncDetails.Library)
            {
                bIsValid = false;
                YS_WARN("Function %s already exists in another Blueprint.  Yarn command names must be unique.", *FuncDetails.Name.ToString())
            }
            // Check name is valid
            const FRegexPattern Pattern{TEXT("^[\\w_][\\w\\d_]*$")};
            FRegexMatcher FunctionNameMatcher(Pattern, FuncDetails.Name.ToString());
            if (!FunctionNameMatcher.FindNext())
            {
                bIsValid = false;
                YS_WARN("Function %s has an invalid name.  Yarn function names must be valid C++ function names.", *FuncDetails.Name.ToString())
            }
            
            // TODO: check function calls Continue() at some point
            
            // Add to function sets
            if (bIsValid)
            {
                LibCommands.FindOrAdd(YarnCommandLibrary).Add(FuncDetails.Name);
                AllCommands.Add(FuncDetails.Name, FuncDetails);
            }
        }
    }
}


void UYarnLibraryRegistry::UpdateFunctions(UBlueprint* YarnFunctionLibrary)
{
    RemoveFunctions(YarnFunctionLibrary);
    ImportFunctions(YarnFunctionLibrary);
}


void UYarnLibraryRegistry::UpdateCommands(UBlueprint* YarnCommandLibrary)
{
    RemoveCommands(YarnCommandLibrary);
    ImportCommands(YarnCommandLibrary);
}


void UYarnLibraryRegistry::RemoveFunctions(UBlueprint* YarnFunctionLibrary)
{
    if (FunctionLibraries.Contains(YarnFunctionLibrary))
    {
        if (LibFunctions.Contains(YarnFunctionLibrary))
        {
            for (auto FuncName : LibFunctions[YarnFunctionLibrary])
            {
                if (AllFunctions.Contains(FuncName) && AllFunctions[FuncName].Library == YarnFunctionLibrary)
                {
                    AllFunctions.Remove(FuncName);
                }
            }
            LibFunctions.Remove(YarnFunctionLibrary);
        }
        FunctionLibraries.Remove(YarnFunctionLibrary);
    }
}


void UYarnLibraryRegistry::RemoveCommands(UBlueprint* YarnCommandLibrary)
{
    if (CommandLibraries.Contains(YarnCommandLibrary))
    {
        if (LibCommands.Contains(YarnCommandLibrary))
        {
            for (auto FuncName : LibCommands[YarnCommandLibrary])
            {
                if (AllCommands.Contains(FuncName) && AllCommands[FuncName].Library == YarnCommandLibrary)
                {
                    AllCommands.Remove(FuncName);
                }
            }
            LibCommands.Remove(YarnCommandLibrary);
        }
        CommandLibraries.Remove(YarnCommandLibrary);
    }
}


void UYarnLibraryRegistry::OnAssetRegistryFilesLoaded()
{
    YS_LOG_FUNCSIG
    bRegistryFilesLoaded = true;
    FindFunctionsAndCommands();
}


void UYarnLibraryRegistry::OnAssetAdded(const FAssetData& AssetData)
{
    YS_LOG_FUNCSIG
    // Ignore this until the registry has finished loading the first time.
    if (!bRegistryFilesLoaded)
        return;

    if (UBlueprint* BP = GetYarnFunctionLibraryBlueprint(AssetData))
    {
        ImportFunctions(BP);
    }
    if (UBlueprint* BP = GetYarnCommandLibraryBlueprint(AssetData))
    {
        ImportCommands(BP);
    }
}


void UYarnLibraryRegistry::OnAssetRemoved(const FAssetData& AssetData)
{
    if (UBlueprint* BP = GetYarnFunctionLibraryBlueprint(AssetData))
    {
        RemoveFunctions(BP);
    }
    if (UBlueprint* BP = GetYarnCommandLibraryBlueprint(AssetData))
    {
        RemoveCommands(BP);
    }
}


void UYarnLibraryRegistry::OnAssetUpdated(const FAssetData& AssetData)
{
    if (UBlueprint* BP = GetYarnFunctionLibraryBlueprint(AssetData))
    {
        UpdateFunctions(BP);
    }
    if (UBlueprint* BP = GetYarnCommandLibraryBlueprint(AssetData))
    {
        UpdateCommands(BP);
    }
}


void UYarnLibraryRegistry::OnAssetRenamed(const FAssetData& AssetData, const FString& String)
{
    // do nothing
}


void UYarnLibraryRegistry::OnStartGameInstance(UGameInstance* GameInstance)
{
    FindFunctionsAndCommands();
}


void UYarnLibraryRegistry::AddStdFunction(const FYarnStdLibFunction& Func)
{
    StdFunctions.Add(Func.Name, Func);
}


void UYarnLibraryRegistry::LoadStdFunctions()
{
    AddStdFunction({TEXT("Number.EqualTo"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2)
        {
            YS_WARN("Number.EqualTo called with incorrect number of parameters (expected 2).")
            return Yarn::Value();
        }
        if (Params[0].GetType() != Yarn::Value::ValueType::NUMBER || Params[1].GetType() != Yarn::Value::ValueType::NUMBER)
        {
            YS_WARN("Number.EqualTo called with incorrect parameter types (expected NUMBER, NUMBER).")
            return Yarn::Value();
        }
        return Yarn::Value(Params[0].GetNumberValue() == Params[1].GetNumberValue());
    }});

    AddStdFunction({TEXT("Number.NotEqualTo"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2)
        {
            YS_WARN("Number.NotEqualTo called with incorrect number of parameters (expected 2).")
            return Yarn::Value();
        }
        if (Params[0].GetType() != Yarn::Value::ValueType::NUMBER || Params[1].GetType() != Yarn::Value::ValueType::NUMBER)
        {
            YS_WARN("Number.NotEqualTo called with incorrect parameter types (expected NUMBER, NUMBER).")
            return Yarn::Value();
        }
        return Yarn::Value(Params[0].GetNumberValue() != Params[1].GetNumberValue());
    }});

    AddStdFunction({TEXT("Number.Add"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2)
        {
            YS_WARN("Number.Add called with incorrect number of parameters (expected 2).")
            return Yarn::Value();
        }
        if (Params[0].GetType() != Yarn::Value::ValueType::NUMBER || Params[1].GetType() != Yarn::Value::ValueType::NUMBER)
        {
            YS_WARN("Number.Add called with incorrect parameter types (expected NUMBER, NUMBER).")
            return Yarn::Value();
        }
        return Yarn::Value(Params[0].GetNumberValue() + Params[1].GetNumberValue());
    }});

    AddStdFunction({TEXT("Number.Minus"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2)
        {
            YS_WARN("Number.Minus called with incorrect number of parameters (expected 2).")
            return Yarn::Value();
        }
        if (Params[0].GetType() != Yarn::Value::ValueType::NUMBER || Params[1].GetType() != Yarn::Value::ValueType::NUMBER)
        {
            YS_WARN("Number.Minus called with incorrect parameter types (expected NUMBER, NUMBER).")
            return Yarn::Value();
        }
        return Yarn::Value(Params[0].GetNumberValue() - Params[1].GetNumberValue());
    }});

    AddStdFunction({TEXT("Number.Divide"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2)
        {
            YS_WARN("Number.Divide called with incorrect number of parameters (expected 2).")
            return Yarn::Value();
        }
        if (Params[0].GetType() != Yarn::Value::ValueType::NUMBER || Params[1].GetType() != Yarn::Value::ValueType::NUMBER)
        {
            YS_WARN("Number.Divide called with incorrect parameter types (expected NUMBER, NUMBER).")
            return Yarn::Value();
        }
        return Yarn::Value(Params[0].GetNumberValue() / Params[1].GetNumberValue());
    }});

    AddStdFunction({TEXT("Number.Multiply"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2)
        {
            YS_WARN("Number.Multiply called with incorrect number of parameters (expected 2).")
            return Yarn::Value();
        }
        if (Params[0].GetType() != Yarn::Value::ValueType::NUMBER || Params[1].GetType() != Yarn::Value::ValueType::NUMBER)
        {
            YS_WARN("Number.Multiply called with incorrect parameter types (expected NUMBER, NUMBER).")
            return Yarn::Value();
        }
        return Yarn::Value(Params[0].GetNumberValue() * Params[1].GetNumberValue());
    }});

    AddStdFunction({TEXT("Number.Modulo"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2)
        {
            YS_WARN("Number.Modulo called with incorrect number of parameters (expected 2).")
            return Yarn::Value();
        }
        if (Params[0].GetType() != Yarn::Value::ValueType::NUMBER || Params[1].GetType() != Yarn::Value::ValueType::NUMBER)
        {
            YS_WARN("Number.Modulo called with incorrect parameter types (expected NUMBER, NUMBER).")
            return Yarn::Value();
        }
        return Yarn::Value(FMath::Fmod(Params[0].GetNumberValue(), Params[1].GetNumberValue()));
    }});

    AddStdFunction({TEXT("Number.UnaryMinus"), 1, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 1)
        {
            YS_WARN("Number.UnaryMinus called with incorrect number of parameters (expected 1).")
            return Yarn::Value();
        }
        if (Params[0].GetType() != Yarn::Value::ValueType::NUMBER)
        {
            YS_WARN("Number.UnaryMinus called with incorrect parameter types (expected NUMBER).")
            return Yarn::Value();
        }
        return Yarn::Value(-Params[0].GetNumberValue());
    }});

    AddStdFunction({TEXT("Number.GreaterThan"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2)
        {
            YS_WARN("Number.GreaterThan called with incorrect number of parameters (expected 2).")
            return Yarn::Value();
        }
        if (Params[0].GetType() != Yarn::Value::ValueType::NUMBER || Params[1].GetType() != Yarn::Value::ValueType::NUMBER)
        {
            YS_WARN("Number.GreaterThan called with incorrect parameter types (expected NUMBER, NUMBER).")
            return Yarn::Value();
        }
        return Yarn::Value(Params[0].GetNumberValue() > Params[1].GetNumberValue());
    }});

    AddStdFunction({TEXT("Number.GreaterThanOrEqualTo"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2)
        {
            YS_WARN("Number.GreaterThanOrEqualTo called with incorrect number of parameters (expected 2).")
            return Yarn::Value();
        }
        if (Params[0].GetType() != Yarn::Value::ValueType::NUMBER || Params[1].GetType() != Yarn::Value::ValueType::NUMBER)
        {
            YS_WARN("Number.GreaterThanOrEqualTo called with incorrect parameter types (expected NUMBER, NUMBER).")
            return Yarn::Value();
        }
        return Yarn::Value(Params[0].GetNumberValue() >= Params[1].GetNumberValue());
    }});

    AddStdFunction({TEXT("Number.LessThan"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2)
        {
            YS_WARN("Number.LessThan called with incorrect number of parameters (expected 2).")
            return Yarn::Value();
        }
        if (Params[0].GetType() != Yarn::Value::ValueType::NUMBER || Params[1].GetType() != Yarn::Value::ValueType::NUMBER)
        {
            YS_WARN("Number.LessThan called with incorrect parameter types (expected NUMBER, NUMBER).")
            return Yarn::Value();
        }
        return Yarn::Value(Params[0].GetNumberValue() < Params[1].GetNumberValue());
    }});

    AddStdFunction({TEXT("Number.LessThanOrEqualTo"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2)
        {
            YS_WARN("Number.LessThanOrEqualTo called with incorrect number of parameters (expected 2).")
            return Yarn::Value();
        }
        if (Params[0].GetType() != Yarn::Value::ValueType::NUMBER || Params[1].GetType() != Yarn::Value::ValueType::NUMBER)
        {
            YS_WARN("Number.LessThanOrEqualTo called with incorrect parameter types (expected NUMBER, NUMBER).")
            return Yarn::Value();
        }
        return Yarn::Value(Params[0].GetNumberValue() <= Params[1].GetNumberValue());
    }});

    AddStdFunction({TEXT("Bool.EqualTo"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2)
        {
            YS_WARN("Bool.EqualTo called with incorrect number of parameters (expected 2).")
            return Yarn::Value();
        }
        if (Params[0].GetType() != Yarn::Value::ValueType::BOOL || Params[1].GetType() != Yarn::Value::ValueType::BOOL)
        {
            YS_WARN("Bool.EqualTo called with incorrect parameter types (expected BOOL, BOOL).")
            return Yarn::Value();
        }
        return Yarn::Value(Params[0].GetBooleanValue() == Params[1].GetBooleanValue());
    }});

    AddStdFunction({TEXT("Bool.NotEqualTo"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2)
        {
           YS_WARN("Bool.NotEqualTo called with incorrect number of parameters (expected 2).")
            return Yarn::Value();
        }
        if (Params[0].GetType() != Yarn::Value::ValueType::BOOL || Params[1].GetType() != Yarn::Value::ValueType::BOOL)
        {
            YS_WARN("Bool.NotEqualTo called with incorrect parameter types (expected BOOL, BOOL).")
            return Yarn::Value();
        }
        return Yarn::Value(Params[0].GetBooleanValue() != Params[1].GetBooleanValue());
    }});

    AddStdFunction({TEXT("Bool.And"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2 || Params[0].GetType() != Yarn::Value::ValueType::BOOL || Params[1].GetType() != Yarn::Value::ValueType::BOOL)
        {
            YS_WARN("Bool.And called with incorrect number of parameters (expected 2) or incorrect parameter types (expected BOOL, BOOL).")
            return Yarn::Value();
        }
        return Yarn::Value(Params[0].GetBooleanValue() && Params[1].GetBooleanValue());
    }});

    AddStdFunction({TEXT("Bool.Or"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2 || Params[0].GetType() != Yarn::Value::ValueType::BOOL || Params[1].GetType() != Yarn::Value::ValueType::BOOL)
        {
            YS_WARN("Bool.Or called with incorrect number of parameters (expected 2) or incorrect parameter types (expected BOOL, BOOL).")
            return Yarn::Value();
        }
        return Yarn::Value(Params[0].GetBooleanValue() || Params[1].GetBooleanValue());
    }});

    AddStdFunction({TEXT("Bool.Xor"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2 || Params[0].GetType() != Yarn::Value::ValueType::BOOL || Params[1].GetType() != Yarn::Value::ValueType::BOOL)
        {
            YS_WARN("Bool.Xor called with incorrect number of parameters (expected 2) or incorrect parameter types (expected BOOL, BOOL).")
            return Yarn::Value();
        }
        return Yarn::Value(Params[0].GetBooleanValue() != Params[1].GetBooleanValue());
    }});

    AddStdFunction({TEXT("Bool.Not"), 1, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 1 || Params[0].GetType() != Yarn::Value::ValueType::BOOL)
        {
            YS_WARN("Bool.Not called with incorrect number of parameters (expected 1) or incorrect parameter types (expected BOOL).")
            return Yarn::Value();
        }
        return Yarn::Value(!Params[0].GetBooleanValue());
    }});

    AddStdFunction({TEXT("String.EqualTo"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2 || Params[0].GetType() != Yarn::Value::ValueType::STRING || Params[1].GetType() != Yarn::Value::ValueType::STRING)
        {
            YS_WARN("String.EqualTo called with incorrect number of parameters (expected 2) or incorrect parameter types (expected STRING, STRING).")
            return Yarn::Value();
        }
        return Yarn::Value(Params[0].GetStringValue() == Params[1].GetStringValue());
    }});

    AddStdFunction({TEXT("String.NotEqualTo"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2 || Params[0].GetType() != Yarn::Value::ValueType::STRING || Params[1].GetType() != Yarn::Value::ValueType::STRING)
        {
            YS_WARN("String.NotEqualTo called with incorrect number of parameters (expected 2) or incorrect parameter types (expected STRING, STRING).")
            return Yarn::Value();
        }
        return Yarn::Value(Params[0].GetStringValue() != Params[1].GetStringValue());
    }});

    AddStdFunction({TEXT("String.Add"), 2, [](TArray<Yarn::Value> Params) -> Yarn::Value
    {
        if (Params.Num() != 2 || Params[0].GetType() != Yarn::Value::ValueType::STRING || Params[1].GetType() != Yarn::Value::ValueType::STRING)
        {
            YS_WARN("String.Add called with incorrect number of parameters (expected 2) or incorrect parameter types (expected STRING, STRING).")
            return Yarn::Value();
        }
        return Yarn::Value(Params[0].GetStringValue() + Params[1].GetStringValue());
    }});
}


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


const TMap<FName, FYarnBlueprintLibFunction>& UYarnLibraryRegistry::GetFunctions() const
{
    return AllFunctions;
}


const TMap<FName, FYarnBlueprintLibFunction>& UYarnLibraryRegistry::GetCommands() const
{
    return AllCommands;
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
    YS_LOG("Project content dir: %s", *FPaths::ProjectContentDir());

    TArray<FAssetData> ExistingAssets = FYarnAssetHelpers::FindAssetsInRegistryByPackagePath<UBlueprint>(FPaths::GetPath("/Game/"));

    for (auto Asset : ExistingAssets)
    {
        // YS_LOG_FUNC("Found asset: %s (%s) %s", *Asset.GetFullName(), *Asset.GetPackage()->GetName(), *Asset.GetAsset()->GetPathName())
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
    // YS_LOG("Function graph: %s", *Func->GetName())
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
        // YS_LOG("Node: %s (Class: %s, Title: %s)", *Node->GetName(), *Node->GetClass()->GetName(), *Node->GetNodeTitle(ENodeTitleType::ListView).ToString())
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
                    
            // YS_LOG("Node is a function entry node with %d pins", Node->Pins.Num())
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
        }
        else if (Node->IsA(UK2Node_FunctionResult::StaticClass()))
        {
            // YS_LOG("Node is a function result node with %d pins", Node->Pins.Num())
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
            if (FuncMeta.bHasMultipleOutParams)
            {
                bIsValid = false;
                YS_WARN("Function %s has multiple return values.  Yarn functions only support a single return value.", *FuncDetails.Name.ToString())
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
            
            // TODO: check function name is valid
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
            
            // TODO: check function name is valid
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
    bRegistryFilesLoaded = true;
    FindFunctionsAndCommands();
}


void UYarnLibraryRegistry::OnAssetAdded(const FAssetData& AssetData)
{
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


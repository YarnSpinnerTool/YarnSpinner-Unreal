// Fill out your copyright notice in the Description page of Project Settings.


#include "YarnLibraryRegistryEditor.h"

#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Internationalization/Regex.h"
#include "Library/YarnCommandLibrary.h"
#include "Library/YarnFunctionLibrary.h"
#include "Library/YarnLibraryRegistry.h"
#include "Library/YarnSpinnerLibraryData.h"
#include "Misc/DefaultValueHelper.h"
#include "Misc/FileHelper.h"
#include "Misc/YarnAssetHelpers.h"
#include "Misc/YSLogging.h"


UYarnLibraryRegistryEditor::UYarnLibraryRegistryEditor()
{
    YS_LOG_FUNCSIG
    const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    // Set up asset registry delegates
    {
        OnAssetRegistryFilesLoadedHandle = AssetRegistry.OnFilesLoaded().AddUObject(this, &UYarnLibraryRegistryEditor::OnAssetRegistryFilesLoaded);
        OnAssetAddedHandle = AssetRegistry.OnAssetAdded().AddUObject(this, &UYarnLibraryRegistryEditor::OnAssetAdded);
        OnAssetRemovedHandle = AssetRegistry.OnAssetRemoved().AddUObject(this, &UYarnLibraryRegistryEditor::OnAssetRemoved);
        OnAssetUpdatedHandle = AssetRegistry.OnAssetUpdated().AddUObject(this, &UYarnLibraryRegistryEditor::OnAssetUpdated);
        OnAssetRenamedHandle = AssetRegistry.OnAssetRenamed().AddUObject(this, &UYarnLibraryRegistryEditor::OnAssetRenamed);
        FWorldDelegates::OnStartGameInstance.AddUObject(this, &UYarnLibraryRegistryEditor::OnStartGameInstance);
    }
}


void UYarnLibraryRegistryEditor::BeginDestroy()
{
    UObject::BeginDestroy();

    IAssetRegistry& AssetRegistry = FAssetRegistryModule::GetRegistry();
    AssetRegistry.OnFilesLoaded().Remove(OnAssetRegistryFilesLoadedHandle);
    AssetRegistry.OnAssetAdded().Remove(OnAssetAddedHandle);
    AssetRegistry.OnAssetRemoved().Remove(OnAssetRemovedHandle);
    AssetRegistry.OnAssetRenamed().Remove(OnAssetRenamedHandle);
}


UBlueprint* UYarnLibraryRegistryEditor::GetYarnFunctionLibraryBlueprint(const FAssetData& AssetData)
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


UBlueprint* UYarnLibraryRegistryEditor::GetYarnCommandLibraryBlueprint(const FAssetData& AssetData)
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


void UYarnLibraryRegistryEditor::SaveYSLS()
{
    YS_LOG_FUNCSIG
    // Write .ysls file
    FString YSLSFileContents = YSLSData.ToJsonString();
    FFileHelper::SaveStringToFile(YSLSFileContents, *FYarnAssetHelpers::YSLSFilePath());
}


void UYarnLibraryRegistryEditor::FindFunctionsAndCommands()
{
    YS_LOG_FUNCSIG
    YS_LOG("Project content dir: %s", *FPaths::ProjectContentDir());

    TArray<FAssetData> ExistingAssets = FYarnAssetHelpers::FindAssetsInRegistryByPackagePath<UBlueprint>(FPaths::GetPath(TEXT("/Game/")));
    // TArray<FAssetData> ExistingAssets = FYarnAssetHelpers::FindAssetsInRegistryEditor<UBlueprint>();
    // TArray<FAssetData> ExistingAssets = FYarnAssetHelpers::FindAssetsInRegistryByPackagePath<UBlueprint>(FPaths::ProjectContentDir());
    // TArray<FAssetData> ExistingAssets = FYarnAssetHelpers::FindAssetsInRegistryByPackagePath<UBlueprint>(FPaths::GetPath(TEXT("/")));

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

    SaveYSLS();
}


void UYarnLibraryRegistryEditor::ExtractFunctionDataFromBlueprintGraph(UBlueprint* YarnFunctionLibrary, UEdGraph* Func, FYarnBlueprintLibFunction& FuncDetails, FYarnBlueprintLibFunctionMeta& FuncMeta, bool bExpectDialogueRunnerParam)
{
    YS_LOG("Function graph: '%s'", *Func->GetName())
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
        YS_LOG("Node: '%s' (Class: '%s', Title: '%s')", *Node->GetName(), *Node->GetClass()->GetName(), *Node->GetNodeTitle(ENodeTitleType::ListView).ToString())
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
                auto& SubCategory = Pin->PinType.PinSubCategory;
                YS_LOG("PinName: '%s', Category: '%s', SubCategory: '%s'", *Pin->PinName.ToString(), *Category.ToString(), *SubCategory.ToString())
                if (bExpectDialogueRunnerParam && Pin->PinName == FName(TEXT("DialogueRunner")) && Category == FName(TEXT("object")))
                {
                    FuncMeta.bHasDialogueRunnerRefParam = true;
                }
                else if (Category == TEXT("bool"))
                {
                    FYarnBlueprintParam Param;
                    Param.Name = Pin->PinName;
                    bool Value = Pin->GetDefaultAsString() == "true";
                    Param.Value = Yarn::FValue(Value);
                    FuncDetails.InParams.Add(Param);
                }
                else if (Category == TEXT("float") || (Category == TEXT("real") && (SubCategory == TEXT("float") || SubCategory == TEXT("double"))))
                {
                    FYarnBlueprintParam Param;
                    Param.Name = Pin->PinName;
                    float Value = 0;
                    FDefaultValueHelper::ParseFloat(Pin->GetDefaultAsString(), Value);
                    Param.Value = Yarn::FValue(Value);
                    FuncDetails.InParams.Add(Param);
                }
                else if (Category == TEXT("string"))
                {
                    FYarnBlueprintParam Param;
                    Param.Name = Pin->PinName;
                    Param.Value = Yarn::FValue(TCHAR_TO_UTF8(*Pin->GetDefaultAsString()));
                    FuncDetails.InParams.Add(Param);
                }
                else if (Category != TEXT("exec"))
                {
                    YS_WARN("Invalid Yarn function input pin type: '%s' ('%s').  Must be bool, float or string.", *Pin->PinType.PinCategory.ToString(), *Pin->PinType.PinSubCategory.ToString())
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
                auto& SubCategory = Pin->PinType.PinSubCategory;
                if (Category == TEXT("bool"))
                {
                    if (FuncDetails.OutParam.IsSet() && (FuncDetails.OutParam->Value.GetType() != Yarn::FValue::EValueType::Bool || FuncDetails.OutParam->Name != Pin->PinName))
                    {
                        // YS_WARN("Function %s has multiple return values or types.  Yarn functions may only have one return value.", *FuncDetails.Name.ToString())
                        FuncMeta.bHasMultipleOutParams = true;
                    }
                    FYarnBlueprintParam Param;
                    Param.Name = Pin->PinName;
                    bool Value = Pin->GetDefaultAsString() == "true";
                    Param.Value = Yarn::FValue(Value);
                    FuncDetails.OutParam = Param;
                }
                // Recent UE5 builds use "real" instead of "float" for float pins and force the subcategory for return values to be "double"
                else if (Category == TEXT("float") || (Category == TEXT("real") && (SubCategory == TEXT("float") || SubCategory == TEXT("double"))))
                {
                    if (FuncDetails.OutParam.IsSet() && (FuncDetails.OutParam->Value.GetType() != Yarn::FValue::EValueType::Number || FuncDetails.OutParam->Name != Pin->PinName))
                    {
                        // YS_WARN("Function %s has multiple return values or types.  Only one is supported.", *FuncDetails.Name.ToString())
                        FuncMeta.bHasMultipleOutParams = true;
                    }
                    FYarnBlueprintParam Param;
                    Param.Name = Pin->PinName;
                    float Value = 0;
                    FDefaultValueHelper::ParseFloat(Pin->GetDefaultAsString(), Value);
                    Param.Value = Yarn::FValue(Value);
                    FuncDetails.OutParam = Param;
                }
                else if (Category == TEXT("string"))
                {
                    if (FuncDetails.OutParam.IsSet() && (FuncDetails.OutParam->Value.GetType() != Yarn::FValue::EValueType::String || FuncDetails.OutParam->Name != Pin->PinName))
                    {
                        // YS_WARN("Function %s has multiple return values or types.  Only one is supported.", *FuncDetails.Name.ToString())
                        FuncMeta.bHasMultipleOutParams = true;
                    }
                    FYarnBlueprintParam Param;
                    Param.Name = Pin->PinName;
                    Param.Value = Yarn::FValue(TCHAR_TO_UTF8(*Pin->GetDefaultAsString()));
                    FuncDetails.OutParam = Param;
                }
                else if (Category != TEXT("exec"))
                {
                    YS_WARN("Invalid Yarn function result pin type: '%s' ('%s').  Must be bool, float or string.", *Pin->PinType.PinCategory.ToString(), *Pin->PinType.PinSubCategory.ToString())
                    FuncMeta.InvalidParams.Add(Pin->PinName.ToString());
                }
            }
        }
        else if (Node->IsA(UK2Node_FunctionTerminator::StaticClass()))
        {
            // YS_LOG("Node is a function terminator node with %d pins", Node->Pins.Num())
        }
        // for (auto Pin : Node->Pins)
        // {
        //     YS_LOG("Pin: %s (Direction: %d, PinType: %s, DefaultValue: %s)", *Pin->GetName(), (int)Pin->Direction, *Pin->PinType.PinCategory.ToString(), *Pin->GetDefaultAsString())
        // }
    }
}


void UYarnLibraryRegistryEditor::AddToYSLSData(FYarnBlueprintLibFunction FuncDetails)
{
    // convert to .ysls
    
    const bool bIsCommand = !FuncDetails.OutParam.IsSet();
    
    FYSLSAction Action;
    Action.YarnName = FuncDetails.Name.ToString();
    Action.DefinitionName = FuncDetails.Name.ToString();
    Action.DefinitionName = FuncDetails.Name.ToString();
    Action.FileName = FuncDetails.Library->GetPathName();
    
    if (bIsCommand)
    {
        Action.Signature = FuncDetails.Name.ToString();
        for (auto Param : FuncDetails.InParams)
        {
            FYSLSParameter Parameter;
            Parameter.Name = Param.Name.ToString();
            Parameter.Type = LexToString(Param.Value.GetType());
            // TODO: add default value support by capturing the default value when inspecting blueprint function
            // Parameter.DefaultValue = Param.Value;
            Action.Parameters.Add(Parameter);
            Action.Signature += " " + Parameter.Name;
        }
        YSLSData.Commands.Add(Action);
    }
    else
    {
        Action.ReturnType = LexToString(FuncDetails.OutParam->Value.GetType());
        Action.Signature = Action.ReturnType + "(";
        for (auto Param : FuncDetails.InParams)
        {
            FYSLSParameter Parameter;
            Parameter.Name = Param.Name.ToString();
            Parameter.Type = LexToString(Param.Value.GetType());
            Action.Parameters.Add(Parameter);
            Action.Signature += Parameter.Name + ", ";
        }
        if (Action.Signature.EndsWith(", "))
        {
            Action.Signature.LeftInline(Action.Signature.Len() - 2);
        }
        Action.Signature += ")";
        YSLSData.Functions.Add(Action);
    }
}

void UYarnLibraryRegistryEditor::ImportFunctions(UBlueprint* YarnFunctionLibrary)
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
        // if (!FuncMeta.bIsPublic)
        // {
        //     YS_LOG("Ignoring private Blueprint function '%s'.", *FuncDetails.Name.ToString())
        // }
        // else
        {
            // Test for valid function
            bool bIsValid = true;
            if (FuncMeta.bHasMultipleOutParams || !FuncDetails.OutParam.IsSet())
            {
                bIsValid = false;
                YS_WARN("Function '%s' must have exactly one return parameter named '%s'.", *FuncDetails.Name.ToString(), *GYSFunctionReturnParamName.ToString())
            }
            if (FuncDetails.OutParam.IsSet() && FuncDetails.OutParam->Name != GYSFunctionReturnParamName)
            {
                bIsValid = false;
                YS_WARN("Function '%s' must have exactly one return parameter named '%s'.", *FuncDetails.Name.ToString(), *GYSFunctionReturnParamName.ToString())
            }
            if (FuncMeta.InvalidParams.Num() > 0)
            {
                bIsValid = false;
                YS_WARN("Function '%s' has invalid parameter types. Yarn functions only support boolean, float and string.", *FuncDetails.Name.ToString())
            }
            // Check name is unique
            if (AllFunctions.Contains(FuncDetails.Name) && AllFunctions[FuncDetails.Name].Library != FuncDetails.Library)
            {
                bIsValid = false;
                YS_WARN("Function '%s' already exists in another Blueprint.  Yarn function names must be unique.", *FuncDetails.Name.ToString())
            }
            // Check name is valid
            const FRegexPattern Pattern{TEXT("^[\\w_][\\w\\d_]*$")};
            FRegexMatcher FunctionNameMatcher(Pattern, FuncDetails.Name.ToString());
            if (!FunctionNameMatcher.FindNext())
            {
                bIsValid = false;
                YS_WARN("Function '%s' has an invalid name.  Yarn function names must be valid C++ function names.", *FuncDetails.Name.ToString())
            }

            // TODO: check function has no async nodes

            // Add to function sets
            if (bIsValid)
            {
                YS_LOG("Adding function '%s' to available YarnSpinner functions.", *FuncDetails.Name.ToString())
                LibFunctions.FindOrAdd(YarnFunctionLibrary).Add(FuncDetails.Name);
                AllFunctions.Add(FuncDetails.Name, FuncDetails);

                AddToYSLSData(FuncDetails);
            }
        }
    }
}


void UYarnLibraryRegistryEditor::ImportCommands(UBlueprint* YarnCommandLibrary)
{
    if (!YarnCommandLibrary)
    {
        // YS_WARN_FUNC("null Blueprint library passed in")
        return;
    }

    // Store a reference to the Blueprint library
    CommandLibraries.Add(YarnCommandLibrary);
    LibCommands.FindOrAdd(YarnCommandLibrary);

    for (UEdGraph* Func : YarnCommandLibrary->EventGraphs)
    {
        FYarnBlueprintLibFunction FuncDetails;
        FYarnBlueprintLibFunctionMeta FuncMeta;

        ExtractFunctionDataFromBlueprintGraph(YarnCommandLibrary, Func, FuncDetails, FuncMeta, true);

        // Test for valid function
        bool bIsValid = true;
        if (!FuncMeta.bHasDialogueRunnerRefParam)
        {
            bIsValid = false;
            // Only show the warning for non-builtin events.
            const FRegexPattern BuiltinEventPattern(TEXT("^[a-zA-Z]+_[A-Z,0-9]+$"));
            FRegexMatcher BuiltinEventMatcher(BuiltinEventPattern, FuncDetails.Name.ToString());
            if (!BuiltinEventMatcher.FindNext())
            {
                YS_WARN("Function '%s' requires a parameter called 'DialogueRunner' which is a reference to a DialogueRunner object in order to be usable as a Yarn command.", *FuncDetails.Name.ToString())
            }
        }
        if (FuncDetails.OutParam.IsSet())
        {
            bIsValid = false;
            YS_WARN("Function '%s' has a return value.  Yarn commands must not return values.", *FuncDetails.Name.ToString())
        }
        if (FuncMeta.InvalidParams.Num() > 0)
        {
            bIsValid = false;
            YS_WARN("Function '%s' has invalid parameter types. Yarn commands only support boolean, float and string.", *FuncDetails.Name.ToString())
        }
        // Check name is unique
        if (AllFunctions.Contains(FuncDetails.Name) && AllFunctions[FuncDetails.Name].Library != FuncDetails.Library)
        {
            bIsValid = false;
            YS_WARN("Function '%s' already exists in another Blueprint.  Yarn command names must be unique.", *FuncDetails.Name.ToString())
        }
        // Check name is valid
        const FRegexPattern Pattern{TEXT("^[\\w_][\\w\\d_]*$")};
        FRegexMatcher FunctionNameMatcher(Pattern, FuncDetails.Name.ToString());
        if (!FunctionNameMatcher.FindNext())
        {
            bIsValid = false;
            YS_WARN("Function '%s' has an invalid name.  Yarn function names must be valid C++ function names.", *FuncDetails.Name.ToString())
        }

        // TODO: check function calls Continue() at some point

        // Add to function sets
        if (bIsValid)
        {
            YS_LOG("Adding command '%s' to available YarnSpinner commands.", *FuncDetails.Name.ToString())
            LibCommands.FindOrAdd(YarnCommandLibrary).Add(FuncDetails.Name);
            AllCommands.Add(FuncDetails.Name, FuncDetails);

            AddToYSLSData(FuncDetails);
        }
    }
}


void UYarnLibraryRegistryEditor::UpdateFunctions(UBlueprint* YarnFunctionLibrary)
{
    RemoveFunctions(YarnFunctionLibrary);
    ImportFunctions(YarnFunctionLibrary);
}


void UYarnLibraryRegistryEditor::UpdateCommands(UBlueprint* YarnCommandLibrary)
{
    RemoveCommands(YarnCommandLibrary);
    ImportCommands(YarnCommandLibrary);
}


void UYarnLibraryRegistryEditor::RemoveFunctions(UBlueprint* YarnFunctionLibrary)
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
                YSLSData.Functions.RemoveAll([FuncName](const FYSLSAction& F) { return F.DefinitionName == FuncName.ToString(); });
            }
            LibFunctions.Remove(YarnFunctionLibrary);
        }
        FunctionLibraries.Remove(YarnFunctionLibrary);
    }
}


void UYarnLibraryRegistryEditor::RemoveCommands(UBlueprint* YarnCommandLibrary)
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
                YSLSData.Commands.RemoveAll([FuncName](const FYSLSAction& F) { return F.DefinitionName == FuncName.ToString(); });
            }
            LibCommands.Remove(YarnCommandLibrary);
        }
        CommandLibraries.Remove(YarnCommandLibrary);
    }
}


void UYarnLibraryRegistryEditor::OnAssetRegistryFilesLoaded()
{
    YS_LOG_FUNCSIG
    bRegistryEditorFilesLoaded = true;
    FindFunctionsAndCommands();
}


void UYarnLibraryRegistryEditor::OnAssetAdded(const FAssetData& AssetData)
{
    // Ignore this until the registry has finished loading the first time.
    if (!bRegistryEditorFilesLoaded)
        return;

    YS_LOG_FUNCSIG

    if (UBlueprint* BP = GetYarnFunctionLibraryBlueprint(AssetData))
    {
        ImportFunctions(BP);
        SaveYSLS();
    }
    if (UBlueprint* BP = GetYarnCommandLibraryBlueprint(AssetData))
    {
        ImportCommands(BP);
        SaveYSLS();
    }
}


void UYarnLibraryRegistryEditor::OnAssetRemoved(const FAssetData& AssetData)
{
    if (UBlueprint* BP = GetYarnFunctionLibraryBlueprint(AssetData))
    {
        RemoveFunctions(BP);
        SaveYSLS();
    }
    if (UBlueprint* BP = GetYarnCommandLibraryBlueprint(AssetData))
    {
        RemoveCommands(BP);
        SaveYSLS();
    }
}


void UYarnLibraryRegistryEditor::OnAssetUpdated(const FAssetData& AssetData)
{
    if (UBlueprint* BP = GetYarnFunctionLibraryBlueprint(AssetData))
    {
        UpdateFunctions(BP);
        SaveYSLS();
    }
    if (UBlueprint* BP = GetYarnCommandLibraryBlueprint(AssetData))
    {
        UpdateCommands(BP);
        SaveYSLS();
    }
}


void UYarnLibraryRegistryEditor::OnAssetRenamed(const FAssetData& AssetData, const FString& String)
{
    if (UBlueprint* BP = GetYarnFunctionLibraryBlueprint(AssetData))
    {
        UpdateFunctions(BP);
        SaveYSLS();
    }
    if (UBlueprint* BP = GetYarnCommandLibraryBlueprint(AssetData))
    {
        UpdateCommands(BP);
        SaveYSLS();
    }
}


void UYarnLibraryRegistryEditor::OnStartGameInstance(UGameInstance* GameInstance)
{
    FindFunctionsAndCommands();
}

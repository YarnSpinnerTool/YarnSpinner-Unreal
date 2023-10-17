// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "YarnSpinnerCore/Value.h"
#include "YarnLibraryRegistry.generated.h"


struct FYSLSAction;
const FName GYSFunctionReturnParamName = TEXT("Out");


USTRUCT()
struct YARNSPINNER_API FYarnBlueprintFuncParam
{
    GENERATED_BODY()

    FName Name;
    
    Yarn::Value Value;
};


USTRUCT()
struct YARNSPINNER_API FYarnBlueprintLibFunction
{
    GENERATED_BODY()

    UBlueprint* Library;
    FName Name;

    TArray<FYarnBlueprintFuncParam> InParams;
    TOptional<FYarnBlueprintFuncParam> OutParam;
};


USTRUCT()
struct YARNSPINNER_API FYarnBlueprintLibFunctionMeta
{
    GENERATED_BODY()

    bool bIsPublic = false;
    bool bIsPure = false;
    bool bIsConst = false;
    bool bHasMultipleOutParams = false;
    // In/out parameters with invalid types
    TArray<FString> InvalidParams;
};


USTRUCT()
struct YARNSPINNER_API FYarnStdLibFunction
{
    GENERATED_BODY()

    FName Name;
    int32 ExpectedParamCount = 0;
    TFunction<Yarn::Value(TArray<Yarn::Value> Params)> Function;
};

/**
 * 
 */
UCLASS()
class YARNSPINNER_API UYarnLibraryRegistry : public UObject
{
    GENERATED_BODY()

public:
    UYarnLibraryRegistry();
    virtual void BeginDestroy() override;

    bool HasFunction(const FName& Name) const;
    int32 GetExpectedFunctionParamCount(const FName& Name) const;
    Yarn::Value CallFunction(const FName& Name, TArray<Yarn::Value> Parameters) const;

private:
    // Blueprints that extend YarnFunctionLibrary
    UPROPERTY()
    TSet<UBlueprint*> FunctionLibraries; 
    UPROPERTY()
    TSet<UBlueprint*> CommandLibraries;

    // A map of blueprints to a list of their function names
    // TMap<UBlueprint*, TArray<FName>> LibFunctions;
    // TMap<UBlueprint*, TArray<FName>> LibCommands;
    // A map of function names to lists of details of implementations
    TMap<FName, FYarnBlueprintLibFunction> AllFunctions;
    TMap<FName, FYarnStdLibFunction> StdFunctions;
    TMap<FName, FYarnBlueprintLibFunction> AllCommands;

    static UBlueprint* GetYarnFunctionLibraryBlueprint(const FAssetData& AssetData);
    static UBlueprint* GetYarnCommandLibraryBlueprint(const FAssetData& AssetData);
    void FindFunctionsAndCommands();
    void AddFunction(const FYSLSAction& Func);
    void AddCommand(const FYSLSAction& Cmd);
    
    void OnStartGameInstance(UGameInstance* GameInstance);

    void AddStdFunction(const FYarnStdLibFunction& Func);
    void LoadStdFunctions();
};

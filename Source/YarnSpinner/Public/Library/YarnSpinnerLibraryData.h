// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YarnSpinnerLibraryData.generated.h"


USTRUCT()
struct YARNSPINNER_API FYSLSParameter
{
    GENERATED_BODY()

    UPROPERTY()
    FString Name;

    // A Yarn type; either 'string', 'number', 'boolean', 'any'.
    UPROPERTY()
    FString Type = "any";

    // Parameter Documentation, used in method signature hinting.
    UPROPERTY()
    FString Documentation;

    // Default value if it exists. Also will make this parameter optional for parameter count validation.
    UPROPERTY()
    FString DefaultValue;

    // Corresponds to the params keyword in C#. Makes this parameter optional, and further parameters will use the information from this parameter.
    // Undefined behavior if true for any parameter except for the last.
    UPROPERTY()
    bool IsParamsArray = false;
};


USTRUCT()
struct YARNSPINNER_API FYSLSAction
{
    GENERATED_BODY()

    // Name of this method in Yarn Spinner scripts
    UPROPERTY()
    FString YarnName;

    // Name of this method in code 
    UPROPERTY()
    FString DefinitionName;

    // Name of the file this method is defined in.
    // Primarily used when 'Deep Command Lookup' is disabled to make sure the source file is still found (doesn't need to be the full path, even 'foo.cs' is helpful).
    UPROPERTY()
    FString FileName;

    // Language id of the method definition.\nMust be 'csharp' to override/merge with methods found in C# files.
    UPROPERTY()
    FString Language = "blueprint";

    // Description that shows up in suggestions and hover tooltips.
    UPROPERTY()
    FString Documentation;

    // Method signature of the method definition. Good way to show parameters, especially if they have default values or are params[].
    UPROPERTY()
    FString Signature;

    // Method parameters.\nNote that if you are overriding information for a method found via parsing code, setting this in json will completely override that parameter information.
    UPROPERTY()
    TArray<FYSLSParameter> Parameters;

    // A Yarn type; either 'string', 'number', 'boolean', 'any'.
    UPROPERTY()
    FString ReturnType = "any";
};


// .ysls file data, as defined in:
// https://github.com/YarnSpinnerTool/YarnSpinner/blob/main/YarnSpinner.LanguageServer/src/Server/Documentation/ysls.schema.json
USTRUCT()
struct YARNSPINNER_API FYarnSpinnerLibraryData
{
    static TOptional<FYarnSpinnerLibraryData> FromJsonString(const FString& JsonString);
    FString ToJsonString();

    GENERATED_BODY()
    // List of commands to make available in Yarn scripts
    UPROPERTY()
    TArray<FYSLSAction> Commands;

    // List of functions to make available in Yarn scripts
    UPROPERTY()
    TArray<FYSLSAction> Functions;
};


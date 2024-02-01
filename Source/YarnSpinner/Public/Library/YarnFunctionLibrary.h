// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YarnLibraryRegistry.h"
#include "YarnSpinnerCore/Value.h"
#include "YarnFunctionLibrary.generated.h"


struct FYarnBlueprintFuncParam;


UCLASS(Blueprintable, ClassGroup = (YarnSpinner))
class YARNSPINNER_API UYarnFunctionLibrary : public UObject
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    UYarnFunctionLibrary();

    static UYarnFunctionLibrary* FromBlueprint(const UBlueprint* Blueprint);

    TOptional<Yarn::FValue> CallFunction(FName FunctionName, TArray<FYarnBlueprintParam> Args, TOptional<FYarnBlueprintParam> ReturnValue);

protected:
    // Called when the game starts or when spawned
    // virtual void BeginPlay() override;

public:
    // Called every frame
    // virtual void Tick(float DeltaTime) override;
};

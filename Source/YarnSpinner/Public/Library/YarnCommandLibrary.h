// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// #include "Engine/Blueprint.h"
#include "YarnSpinnerCore/Value.h"
#include "YarnCommandLibrary.generated.h"


struct FYarnBlueprintFuncParam;


UCLASS(Blueprintable, ClassGroup = (YarnSpinner))
class YARNSPINNER_API UYarnCommandLibrary : public UObject
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    UYarnCommandLibrary();

    TOptional<Yarn::Value> CallFunction(FName FunctionName, TArray<FYarnBlueprintFuncParam> Args, TOptional<FYarnBlueprintFuncParam> ReturnValue);

protected:
    // Called when the game starts or when spawned
    // virtual void BeginPlay() override;

public:
    // Called every frame
    // virtual void Tick(float DeltaTime) override;
};

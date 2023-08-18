﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "YarnSpinnerCore/Value.h"
#include "YarnFunctionLibrary.generated.h"


struct FYarnBlueprintArg
{
    FName Name;
    Yarn::Value Value;
};



UCLASS(Blueprintable, ClassGroup = (YarnSpinner))
class YARNSPINNER_API AYarnFunctionLibrary : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AYarnFunctionLibrary();

    TOptional<Yarn::Value> CallFunction(FName FunctionName, TArray<FYarnBlueprintArg> Args, TOptional<FYarnBlueprintArg> ReturnValue);

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
};
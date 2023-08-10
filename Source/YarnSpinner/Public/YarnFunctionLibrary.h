// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "YarnFunctionLibrary.generated.h"


UCLASS(Blueprintable, ClassGroup = (YarnSpinner))
class YARNSPINNER_API AYarnFunctionLibrary : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AYarnFunctionLibrary();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
};

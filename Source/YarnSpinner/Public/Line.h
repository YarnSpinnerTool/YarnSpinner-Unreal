// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Line.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class YARNSPINNER_API ULine : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Yarn Spinner")
    FName LineID;

    UPROPERTY(BlueprintReadWrite, Category="Yarn Spinner")
    FText DisplayText;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Line.h"
#include "CoreMinimal.h"
#include "Option.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class YARNSPINNER_API UOption : public UObject
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite)
	ULine *Line;

	// Indicates whether the line condition on this option evaluated to true (or
	// if no line condition was present.)
	UPROPERTY(BlueprintReadWrite)
	bool bIsAvailable;

	int OptionID;
};

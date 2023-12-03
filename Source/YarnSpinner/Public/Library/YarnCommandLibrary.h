// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YarnCommandLibrary.generated.h"


struct FYarnBlueprintParam;


UCLASS(Blueprintable, ClassGroup = (YarnSpinner))
class YARNSPINNER_API UYarnCommandLibrary : public UObject
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    UYarnCommandLibrary();

    static UYarnCommandLibrary* FromBlueprint(const UBlueprint* Blueprint);

    void CallCommand(FName CommandName, TSoftObjectPtr<class ADialogueRunner> DialogueRunner, TArray<FYarnBlueprintParam> Args);

protected:
    // Called when the game starts or when spawned
    // virtual void BeginPlay() override;

public:
    // Called every frame
    // virtual void Tick(float DeltaTime) override;

private:
    static void ContinueDialogue(const TSoftObjectPtr<class ADialogueRunner>& DialogueRunner);
};

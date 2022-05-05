// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialogueRunner.generated.h"

UCLASS()
class YARNSPINNER_API ADialogueRunner : public AActor
{
    GENERATED_BODY()
    
public:
    // Sets default values for this actor's properties
    ADialogueRunner();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
    
    UFUNCTION(BlueprintNativeEvent, Category="Dialogue Runner")
    void OnDialogueStarted();
    
    UFUNCTION(BlueprintNativeEvent, Category="Dialogue Runner")
    void OnDialogueEnded();
    
    UFUNCTION(BlueprintNativeEvent, Category="Dialogue Runner")
    void OnRunLine(class ULine* line);
    
    UFUNCTION(BlueprintCallable)
    void StartDialogue(FName nodeName);
    
    UFUNCTION(BlueprintCallable)
    void ContinueDialogue();
    
    UFUNCTION(BlueprintCallable)
    void SelectOption(int optionID);
    
private:
    UPROPERTY(VisibleInstanceOnly)
    int32 currentContentIndex = 0;
    
    class ULine* GetFakeLine(int index);
    
    class ULine* MakeFakeLine(FName lineID);

};

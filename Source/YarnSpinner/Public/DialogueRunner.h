// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "YarnAsset.h"
#include "YarnSpinnerCore/VirtualMachine.h"
#include "YarnSpinnerCore/Library.h"
#include "YarnSpinnerCore/Common.h"
#include "DialogueRunner.generated.h"

UCLASS()
class YARNSPINNER_API ADialogueRunner : public AActor, public Yarn::ILogger, public Yarn::IVariableStorage
{
    GENERATED_BODY()
    
public:
    // Sets default values for this actor's properties
    ADialogueRunner();

protected:
    virtual void PreInitializeComponents() override;
    
public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintNativeEvent, Category="Dialogue Runner")
    void OnDialogueStarted();
    
    UFUNCTION(BlueprintNativeEvent, Category="Dialogue Runner")
    void OnDialogueEnded();
    
    UFUNCTION(BlueprintNativeEvent, Category="Dialogue Runner")
    void OnRunLine(class ULine* line);

    UFUNCTION(BlueprintNativeEvent, Category="Dialogue Runner")
    void OnRunOptions(const TArray<class UOption*>& options);

    UFUNCTION(BlueprintNativeEvent, Category="Dialogue Runner")
    void OnRunCommand(const FString& command);
    
    UFUNCTION(BlueprintCallable, Category="Dialogue Runner")
    void StartDialogue(FName nodeName);
    
    UFUNCTION(BlueprintCallable, Category="Dialogue Runner")
    void ContinueDialogue();
    
    UFUNCTION(BlueprintCallable, Category="Dialogue Runner")
    void SelectOption(UOption* option);
    
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Dialogue Runner")
    UYarnAsset* yarnAsset;

private:
    TUniquePtr<Yarn::VirtualMachine> VirtualMachine;

    TUniquePtr<Yarn::Library> Library;

    // ILogger
    virtual void Log(std::string message, Type severity = Type::INFO) override;

    // IVariableStorage
    virtual void SetValue(std::string name, bool value) override;
    virtual void SetValue(std::string name, float value) override;
    virtual void SetValue(std::string name, std::string value) override;

    virtual bool HasValue(std::string name) override;
    virtual Yarn::Value GetValue(std::string name) override;

    virtual void ClearValue(std::string name) override;
};

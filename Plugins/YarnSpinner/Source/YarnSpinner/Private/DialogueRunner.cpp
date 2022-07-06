// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueRunner.h"
#include "Line.h"
#include "YarnSpinner.h"
#include "YarnSpinnerCore/VirtualMachine.h"
//#include "StaticParty.h"

// Sets default values
ADialogueRunner::ADialogueRunner()
{
     // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADialogueRunner::PreInitializeComponents()
{
    Super::PreInitializeComponents();


    if (!yarnAsset) {
        UE_LOG(LogYarnSpinner, Log, TEXT("DialogueRunner can't initialize, because it doesn't have a Yarn Asset."));
        return;
    }

    Yarn::Program program;

    bool parseSuccess = program.ParsePartialFromArray(yarnAsset->Data.GetData(), yarnAsset->Data.Num());

    if (!parseSuccess) {
        UE_LOG(LogYarnSpinner, Warning, TEXT("DialogueRunner can't initialize, because its Yarn Asset failed to parse."));
        return;
    }

    // Create the Library
    this->Library = TUniquePtr<Yarn::Library>(new Yarn::Library(*this));

    // Create the VirtualMachine, supplying it with the loaded Program and
    // configuring it to use our library, plus use this ADialogueRunner as the
    // logger and the variable storage
    this->VirtualMachine = TUniquePtr<Yarn::VirtualMachine>(new Yarn::VirtualMachine(program, *(this->Library), *this, *this));
    
}

// Called every frame
void ADialogueRunner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void ADialogueRunner::OnDialogueStarted_Implementation() {
    // default = no-op
}

void ADialogueRunner::OnDialogueEnded_Implementation() {
    // default = no-op
}

void ADialogueRunner::OnRunLine_Implementation(ULine* line) {
    // default = no-op
}

/** Starts running dialogue from the given node name. */
void ADialogueRunner::StartDialogue(FName nodeName) {
    currentContentIndex = 0;
    OnDialogueStarted();
    ContinueDialogue();
}

/** Continues running the current dialogue, producing either lines, options, commands, or a dialogue-end signal. */
void ADialogueRunner::ContinueDialogue() {
    ULine* line = GetFakeLine(currentContentIndex++);
    
    if (line) {
        // We have a line to show! Deliver it.
        OnRunLine(line);
    } else {
        // No more content!
        OnDialogueEnded();
    }
}

/** Indicates to the dialogue runner that an option was selected. */
void ADialogueRunner::SelectOption(int optionID) {
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Got option %d"), optionID));

    ContinueDialogue();
}

// ---- FAKE CONTENT - REMOVE THESE ASAP

ULine* ADialogueRunner::GetFakeLine(int index) {
    switch (index) {
        case 0:
            return MakeFakeLine(FName(TEXT("This feels weird!")));
        case 1:
            return MakeFakeLine(FName(TEXT("The world doesn't seem right!")));
        case 2:
            return MakeFakeLine(FName(TEXT("It feels a bit... Unreal!")));
        case 3:
            return MakeFakeLine(FName(TEXT("Engine!")));
        default:
            return NULL;
    }
}

ULine* ADialogueRunner::MakeFakeLine(FName lineID) {
    auto line = NewObject<ULine>(this);
    line->LineID = lineID;
    return line;
}

void ADialogueRunner::Log(std::string message, Type severity) {

    switch (severity) {
        case Type::INFO:
            UE_LOG(LogYarnSpinner, Log, TEXT("YarnSpinner: %s"), message.c_str());
            break;
        case Type::WARNING:
            UE_LOG(LogYarnSpinner, Warning, TEXT("YarnSpinner: %s"), message.c_str());
            break;
        case Type::ERROR:
            UE_LOG(LogYarnSpinner, Error, TEXT("YarnSpinner: %s"), message.c_str());
            break;
        }

}

void ADialogueRunner::SetValue(std::string name, bool value) {
    UE_LOG(LogYarnSpinner, Error, TEXT("YarnSpinner: set %s to bool %i"), name.c_str(), value);
}

void ADialogueRunner::SetValue(std::string name, float value) {
    UE_LOG(LogYarnSpinner, Error, TEXT("YarnSpinner: set %s to float %f"), name.c_str(), value);
}

void ADialogueRunner::SetValue(std::string name, std::string value) {
    UE_LOG(LogYarnSpinner, Error, TEXT("YarnSpinner: set %s to string \"%s\""), name.c_str(), value.c_str());
}

bool ADialogueRunner::HasValue(std::string name) {
    return false;
}

Yarn::Value ADialogueRunner::GetValue(std::string name) {
    UE_LOG(LogYarnSpinner, Error, TEXT("YarnSpinner: get %s"), name.c_str());

    return Yarn::Value(0);
}

void ADialogueRunner::ClearValue(std::string name) {

}

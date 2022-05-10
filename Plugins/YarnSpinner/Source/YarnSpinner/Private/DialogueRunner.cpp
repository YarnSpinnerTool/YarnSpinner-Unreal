// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueRunner.h"
#include "Line.h"
//#include "StaticParty.h"

// Sets default values
ADialogueRunner::ADialogueRunner()
{
     // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADialogueRunner::BeginPlay()
{
    Super::BeginPlay();

    
   GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("hi this is new lol"));
//    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("HelloFromStaticParty says %s"), *(StaticPartyMethods::GimmeSomeJSON())));
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
            return MakeFakeLine(FName(TEXT("Line one!!")));
        case 1:
            return MakeFakeLine(FName(TEXT("Line two!!")));
        case 2:
            return MakeFakeLine(FName(TEXT("Line three!!")));
        default:
            return NULL;
    }
}

ULine* ADialogueRunner::MakeFakeLine(FName lineID) {
    auto line = NewObject<ULine>(this);
    line->LineID = lineID;
    return line;
}


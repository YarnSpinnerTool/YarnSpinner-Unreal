// Fill out your copyright notice in the Description page of Project Settings.

#include "Option.h"
#include "YarnSpinner.h"
#include "DialogueRunner.h"
#include "Misc/YSLogging.h"


void UOption::SelectOption() {
    if (IsValid(this->SourceDialogueRunner) == false) {
        UE_LOG(LogYarnSpinner, Error, TEXT("Can't select option %s: source dialogue runner isn't valid"), *Line->LineID.ToString());
        return;
    }

    this->SourceDialogueRunner->SelectOption(this);
}

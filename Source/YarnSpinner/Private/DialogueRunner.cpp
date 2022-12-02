// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueRunner.h"
#include "Line.h"
#include "Option.h"
#include "YarnSpinner.h"
#include "YarnSpinnerCore/VirtualMachine.h"
//#include "StaticParty.h"

static void GetDisplayTextForLine(ULine* line, Yarn::Line& yarnLine, UYarnAsset* yarnAsset) {
    // FIXME: Currently, we store the text of lines directly in the
    // YarnAsset. This will eventually be replaced with string tables.

    FName lineID = FName(yarnLine.LineID.c_str());

    FString* maybeNonLocalisedDisplayText = yarnAsset->Lines.Find(lineID);
    if (maybeNonLocalisedDisplayText) {

        FString nonLocalisedDisplayText = *maybeNonLocalisedDisplayText;

        // Apply substitutions
        TArray<FStringFormatArg> formatArguments;

        for (auto substitution : yarnLine.Substitutions)
        {
            formatArguments.Add(FStringFormatArg(UTF8_TO_TCHAR(substitution.c_str())));
        }

        FString textWithSubstitutions = FString::Format(*nonLocalisedDisplayText, formatArguments);

        line->DisplayText = FText::FromString(textWithSubstitutions);
    } else {
        line->DisplayText = FText::FromString(TEXT("(missing line!)"));
    }
}

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
        UE_LOG(LogYarnSpinner, Error, TEXT("DialogueRunner can't initialize, because it doesn't have a Yarn Asset."));
        return;
    }

    Yarn::Program program;

    bool parseSuccess = program.ParsePartialFromArray(yarnAsset->Data.GetData(), yarnAsset->Data.Num());

    if (!parseSuccess) {
        UE_LOG(LogYarnSpinner, Error, TEXT("DialogueRunner can't initialize, because its Yarn Asset failed to load."));
        return;
    }

    // Create the Library
    this->Library = TUniquePtr<Yarn::Library>(new Yarn::Library(*this));

    // Create the VirtualMachine, supplying it with the loaded Program and
    // configuring it to use our library, plus use this ADialogueRunner as the
    // logger and the variable storage
    this->VirtualMachine = TUniquePtr<Yarn::VirtualMachine>(new Yarn::VirtualMachine(program, *(this->Library), *this, *this));

    this->VirtualMachine->LineHandler = [this](Yarn::Line &line) {
        UE_LOG(LogYarnSpinner, Log, TEXT("Received line %s"), UTF8_TO_TCHAR(line.LineID.c_str()));

        // Get the Yarn line struct, and make a ULine out of it to use
        ULine* lineObject = NewObject<ULine>(this);
        lineObject->LineID = FName(line.LineID.c_str());

        GetDisplayTextForLine(lineObject, line, yarnAsset);

        OnRunLine(lineObject);
    };

    this->VirtualMachine->OptionsHandler = [this](Yarn::OptionSet &optionSet)
    {
        UE_LOG(LogYarnSpinner, Log, TEXT("Received %i options"), optionSet.Options.size());

        // Build a TArray for every option in this OptionSet
        TArray<UOption *> options;

        for (auto option : optionSet.Options) {
            UE_LOG(LogYarnSpinner, Log, TEXT("- %i: %s"), option.ID, UTF8_TO_TCHAR(option.Line.LineID.c_str()));

            UOption *opt = NewObject<UOption>(this);
            opt->OptionID = option.ID;

            opt->Line = NewObject<ULine>(opt);
            opt->Line->LineID = FName(option.Line.LineID.c_str());

            GetDisplayTextForLine(opt->Line, option.Line, yarnAsset);

            opt->bIsAvailable = option.IsAvailable;

            opt->SourceDialogueRunner = this;

            options.Add(opt);
        }

        OnRunOptions(options);
        
    };

    this->VirtualMachine->CommandHandler = [this](Yarn::Command &command)
    {
        UE_LOG(LogYarnSpinner, Log, TEXT("Received command \"%s\""), UTF8_TO_TCHAR(command.Text.c_str()));

        FString commandText = FString(UTF8_TO_TCHAR(command.Text.c_str()));

        TArray<FString> commandElements;
        commandText.ParseIntoArray(commandElements, TEXT(" "));

        if (commandElements.Num() == 0) {
            TArray<FString> emptyParameters;
            UE_LOG(LogYarnSpinner, Error, TEXT("Command received, but was unable to parse it."));
            OnRunCommand(FString("(unknown)"), emptyParameters);
            return;
        }

        FString commandName = commandElements[0];
        commandElements.RemoveAt(0);

        OnRunCommand(commandName, commandElements);

    };

    this->VirtualMachine->NodeStartHandler = [this](std::string nodeName)
    {
        UE_LOG(LogYarnSpinner, Log, TEXT("Received node start \"%s\""), UTF8_TO_TCHAR(nodeName.c_str()));
    };

    this->VirtualMachine->NodeCompleteHandler = [this](std::string nodeName)
    {
        UE_LOG(LogYarnSpinner, Log, TEXT("Received node complete \"%s\""), UTF8_TO_TCHAR(nodeName.c_str()));
    };

    this->VirtualMachine->DialogueCompleteHandler = [this]()
    {
        UE_LOG(LogYarnSpinner, Log, TEXT("Received dialogue complete"));
        OnDialogueEnded();
    };
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
    // default = log and immediately continue
    UE_LOG(LogYarnSpinner, Warning, TEXT("DialogueRunner received line with ID \"%s\". Implement OnRunLine to customise its behaviour."), *line->LineID.ToString());
    ContinueDialogue();
}

void ADialogueRunner::OnRunOptions_Implementation(const TArray<class UOption*>& options) {
    // default = log and choose the first option
    UE_LOG(LogYarnSpinner, Warning, TEXT("DialogueRunner received %i options. Choosing the first one by default. Implement OnRunOptions to customise its behaviour."), options.Num());

    SelectOption(options[0]);
}

void ADialogueRunner::OnRunCommand_Implementation(const FString& command, const TArray<FString>& parameters) {
    // default = no-op
    UE_LOG(LogYarnSpinner, Warning, TEXT("DialogueRunner received command \"%s\". Implement OnRunCommand to customise its behaviour."), *command);
    ContinueDialogue();
}

/** Starts running dialogue from the given node name. */
void ADialogueRunner::StartDialogue(FName nodeName) {

    if (VirtualMachine.IsValid() == false) {
        UE_LOG(LogYarnSpinner, Error, TEXT("DialogueRunner can't start node %s, because it failed to load a Yarn asset."), *nodeName.ToString());
        return;
    }

    bool nodeSelected = VirtualMachine->SetNode(TCHAR_TO_UTF8(*nodeName.ToString()));

    if (nodeSelected) {
        OnDialogueStarted();
        ContinueDialogue();
    } else {
        UE_LOG(LogYarnSpinner, Error, TEXT("DialogueRunner can't start node %s, because a node with that name was not found."), *nodeName.ToString());
        return;

    }

}

/** Continues running the current dialogue, producing either lines, options, commands, or a dialogue-end signal. */
void ADialogueRunner::ContinueDialogue() {

    if (this->VirtualMachine->GetCurrentExecutionState() == Yarn::VirtualMachine::ExecutionState::ERROR) {
        UE_LOG(LogYarnSpinner, Error, TEXT("VirtualMachine is in an error state, and cannot continue running."));
        return;
    }

    this->VirtualMachine->Continue();

    Yarn::VirtualMachine::ExecutionState state = this->VirtualMachine->GetCurrentExecutionState();

    if (state == Yarn::VirtualMachine::ExecutionState::ERROR)
    {
        UE_LOG(LogYarnSpinner, Error, TEXT("VirtualMachine encountered an error."));
        return;
    }
}

/** Indicates to the dialogue runner that an option was selected. */
void ADialogueRunner::SelectOption(UOption* option) {

    Yarn::VirtualMachine::ExecutionState state = this->VirtualMachine->GetCurrentExecutionState();    

    if (state != Yarn::VirtualMachine::ExecutionState::WAITING_ON_OPTION_SELECTION) {
        UE_LOG(LogYarnSpinner, Error, TEXT("Dialogue Runner received a call to SelectOption, but it wasn't expecting a selection!"));
        return;
    }

    UE_LOG(LogYarnSpinner, Log, TEXT("Selected option %i (%s)"), option->OptionID, *option->Line->LineID.ToString());

    this->VirtualMachine->SetSelectedOption(option->OptionID);

    ContinueDialogue();
}

void ADialogueRunner::Log(std::string message, Type severity) {

    auto messageText = UTF8_TO_TCHAR(message.c_str());

    switch (severity) {
        case Type::INFO:
            UE_LOG(LogYarnSpinner, Log, TEXT("YarnSpinner: %s"), messageText);
            break;
        case Type::WARNING:
            UE_LOG(LogYarnSpinner, Warning, TEXT("YarnSpinner: %s"), messageText);
            break;
        case Type::ERROR:
            UE_LOG(LogYarnSpinner, Error, TEXT("YarnSpinner: %s"), messageText);
            break;
        }

}

void ADialogueRunner::SetValue(std::string name, bool value) {
    UE_LOG(LogYarnSpinner, Error, TEXT("Setting Yarn variables is not currently supported. (Attempted to set set %s to bool %i)"), UTF8_TO_TCHAR(name.c_str()), value);
}

void ADialogueRunner::SetValue(std::string name, float value) {
    UE_LOG(LogYarnSpinner, Error, TEXT("Setting Yarn variables is not currently supported.  (Attempted to set %s to float %f)"), UTF8_TO_TCHAR(name.c_str()), value);
}

void ADialogueRunner::SetValue(std::string name, std::string value) {
    UE_LOG(LogYarnSpinner, Error, TEXT("Setting Yarn variables is not currently supported. (Attempted to set %s to string \"%s\")"), 
        UTF8_TO_TCHAR(name.c_str()), 
        UTF8_TO_TCHAR(value.c_str()));
}

bool ADialogueRunner::HasValue(std::string name) {
    return false;
}

Yarn::Value ADialogueRunner::GetValue(std::string name) {
    UE_LOG(LogYarnSpinner, Error, TEXT("Getting Yarn variables is not currently supported. (%s)"), UTF8_TO_TCHAR(name.c_str()));

    return Yarn::Value(0);
}

void ADialogueRunner::ClearValue(std::string name) {
    UE_LOG(LogYarnSpinner, Error, TEXT("Clearing Yarn variables is not currently supported. (%s)"), UTF8_TO_TCHAR(name.c_str()));

}

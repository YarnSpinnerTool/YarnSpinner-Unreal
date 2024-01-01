#pragma once

#include <string>
#include <memory>
#include "YarnSpinnerCore/yarn_spinner.pb.h"

#include "YarnSpinnerCore/Common.h"
#include "YarnSpinnerCore/Library.h"
#include "YarnSpinnerCore/State.h"
#include "Value.h"

#include <functional>

namespace Yarn
{

    class YARNSPINNER_API IVariableStorage
    {
    public:
        virtual ~IVariableStorage() = default;
        
        virtual void SetValue(const FString& name, bool value) = 0;
        virtual void SetValue(const FString& name, float value) = 0;
        virtual void SetValue(const FString& name, const FString& value) = 0;

        virtual bool HasValue(const FString& name) = 0;
        virtual FValue GetValue(const FString& name) = 0;

        virtual void ClearValue(const FString& name) = 0;
    };

    // Function handler delegate definitions
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnLine, const Line&);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnOptions, const OptionSet&);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnCommand, const Command&);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnNodeStart, const FString&);
    DECLARE_MULTICAST_DELEGATE_OneParam(FOnNodeComplete, const FString&);
    DECLARE_MULTICAST_DELEGATE(FOnDialogueComplete);
    DECLARE_DELEGATE_RetVal_OneParam(bool, FOnCheckFunctionExist, const FString&);
    DECLARE_DELEGATE_RetVal_OneParam(int, FOnGetFunctionParamNum, const FString&);
    DECLARE_DELEGATE_RetVal_TwoParams(FValue, FOnCallFunction, const FString&, const TArray<FValue>&);

    class YARNSPINNER_API VirtualMachine
    {
    public:
        enum ExecutionState
        {
            /// The VirtualMachine is not running a node.
            STOPPED,

            /// The VirtualMachine is waiting on option selection. Call
            /// SetSelectedOption before calling Continue.
            WAITING_ON_OPTION_SELECTION,

            /// The VirtualMachine has finished delivering content to the client
            /// game, and is waiting for Continue to be called.
            WAITING_FOR_CONTINUE,

            /// The VirtualMachine is delivering a line, options, or a
            /// commmand to the client game.
            DELIVERING_CONTENT,

            /// The VirtualMachine is in the middle of executing code.
            RUNNING,

            /// The VirtualMachine has encountered an error and cannot continue executing.
            ERROR
        };

    private:
        Yarn::Program program;

        Yarn::Node currentNode;

        FString currentNodeName;

        State state;

        ExecutionState executionState;

        Library &library;
        IVariableStorage &variableStorage;

    public:
        VirtualMachine(Yarn::Program program, Library &library, IVariableStorage &variableStorage);
        ~VirtualMachine();

        void SetProgram(Yarn::Program program);
        const Yarn::Program &GetProgram();

        bool SetNode(const FString& NodeName);
        const char *GetCurrentNodeName();

        ExecutionState GetCurrentExecutionState();

        // Begins or continues execution of the virtual machine.
        bool Continue();

        // Function handlers
        FOnLine OnLine;
        FOnOptions OnOptions;
        FOnCommand OnCommand;
        FOnNodeStart OnNodeStart;
        FOnNodeComplete OnNodeComplete;
        FOnDialogueComplete OnDialogueComplete;
        FOnCheckFunctionExist OnCheckFunctionExist;
        FOnGetFunctionParamNum OnGetFunctionParamNum;
        FOnCallFunction OnCallFunction;

        void SetSelectedOption(int selectedOptionIndex);

        static FString ExpandSubstitutions(const FString& TemplateString, const TArray<FString>& Substitutions);

    private:
        void SetCurrentExecutionState(ExecutionState state);
        bool CheckCanContinue();
        bool RunInstruction(Yarn::Instruction &instruction);
        int FindInstructionPointForLabel(const FString& Label);
    };
}

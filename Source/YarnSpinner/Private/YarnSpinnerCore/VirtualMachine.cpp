#include "YarnSpinnerCore/VirtualMachine.h"

#include <regex>
#include <string>
#include <string>

#include "CoreMinimal.h"
#include "Misc/YSLogging.h"


namespace Yarn
{
    VirtualMachine::VirtualMachine(Yarn::Program program, Library& library, IVariableStorage& variableStorage)
        : program(program),
          state(State()),
          executionState(STOPPED),
          library(library),
          variableStorage(variableStorage)
    {
        // Add the 'visited' and 'visited_count' functions, which query the variable
        // storage for information about how many times a node has been visited.
        library.AddFunction(
            "visited",
            TYarnFunction<bool>::CreateLambda(
            [&variableStorage](const TArray<FValue>& Values)
            {
                const FString NodeName = Values[0].GetValue<FString>();
                const FString VisitTrackingVariable = Library::GenerateUniqueVisitedVariableForNode(NodeName);
                return variableStorage.HasValue(VisitTrackingVariable) ?
                    variableStorage.GetValue(VisitTrackingVariable).GetValue<double>() > 0 :
                    false;
            }),
            1);

        library.AddFunction(
            "visited_count",
            TYarnFunction<float>::CreateLambda(
            [&variableStorage](const TArray<FValue>& Values)
            {
                const FString NodeName = Values[0].GetValue<FString>();
                const FString VisitTrackingVariable = Library::GenerateUniqueVisitedVariableForNode(NodeName);
                return variableStorage.HasValue(VisitTrackingVariable) ?
                    static_cast<int>(variableStorage.GetValue(VisitTrackingVariable).GetValue<double>()) :
                    0;
            }),
            1);
    }


    VirtualMachine::~VirtualMachine()
    {
    }


    void VirtualMachine::SetProgram(Yarn::Program newProgram)
    {
        this->program = newProgram;
        SetCurrentExecutionState(STOPPED);
        state.programCounter = 0;
    }


    const Yarn::Program& VirtualMachine::GetProgram()
    {
        return this->program;
    }


    bool VirtualMachine::SetNode(const FString& NodeName)
    {
        if (program.nodes().contains(TCHAR_TO_UTF8(*NodeName)) == false)
        {
            YS_ERR("No node named %s has been loaded.", *NodeName);
            return false;
        }

        YS_LOG("Running node %s", *NodeName);

        currentNode = program.nodes().at(TCHAR_TO_UTF8(*NodeName));

        // Clear our State and return to the Stopped execution state
        state = State();
        SetCurrentExecutionState(ExecutionState::STOPPED);

        state.currentNodeName = NodeName;

        OnNodeStart.Broadcast(NodeName);

        return true;
    }


    const char* VirtualMachine::GetCurrentNodeName()
    {
        return currentNode.name().c_str();
    }


    VirtualMachine::ExecutionState VirtualMachine::GetCurrentExecutionState()
    {
        return executionState;
    }


    void VirtualMachine::SetCurrentExecutionState(VirtualMachine::ExecutionState newState)
    {
        executionState = newState;
        if (executionState == STOPPED)
        {
            // We've stopped; clear our state.
            state = State();
        }
    }


    bool VirtualMachine::Continue()
    {
        // Perform a safety check to ensure that we're in a ready state to continue
        if (CheckCanContinue() == false)
        {
            return false;
        }

        if (executionState == ExecutionState::DELIVERING_CONTENT)
        {
            // We were delivering a line, option set, or command, and the client has
            // called Continue() on us. We're still inside the stack frame of the
            // client callback, so to avoid recursion, we'll note that our state has
            // changed back to RUNNING; when we've left the callback, we'll continue
            // executing instructions.
            SetCurrentExecutionState(RUNNING);
            return true;
        }

        SetCurrentExecutionState(RUNNING);

        while (GetCurrentExecutionState() == RUNNING)
        {
            Yarn::Instruction currentInstruction = currentNode.instructions().at(state.programCounter);

            bool successfullyRanInstruction = RunInstruction(currentInstruction);

            if (!successfullyRanInstruction)
            {
                // Stop immediately - we ran into a problem when executing our
                // instruction
                SetCurrentExecutionState(VirtualMachine::ExecutionState::ERROR);
                return false;
            }

            state.programCounter += 1;

            if (state.programCounter >= currentNode.instructions_size() && GetCurrentExecutionState() != STOPPED)
            {
                OnNodeComplete.Broadcast(currentNode.name().c_str());
                SetCurrentExecutionState(STOPPED);
                OnDialogueComplete.Broadcast();
                YS_LOG("Run complete.");
            }
        }

        return true;
    }


    bool VirtualMachine::RunInstruction(Yarn::Instruction& instruction)
    {
        TStringBuilder<NAME_SIZE> StrBuilder;

        StrBuilder << Instruction_OpCode_Name(instruction.opcode()).c_str();

        for (auto operand : instruction.operands())
        {
            StrBuilder << " ";
            switch (operand.value_case())
            {
            case Yarn::Operand::kBoolValue:
                StrBuilder << (operand.bool_value() ? "true" : "false");
                break;
            case Yarn::Operand::kFloatValueFieldNumber:
                StrBuilder << FString::SanitizeFloat(operand.float_value());
                break;
            case Yarn::Operand::kStringValue:
                StrBuilder << operand.string_value().c_str();
                break;
            default:
                StrBuilder << "(unknown operand type!)";
            }
        }

        YS_LOG("%s", StrBuilder.ToString());
        
        switch (instruction.opcode())
        {
        case Yarn::Instruction_OpCode_RUN_LINE:
            {
                // Fetch line ID
                auto lineID = instruction.operands(0).string_value();

                // Build line structs
                Line line = Line();
                line.LineID = lineID.c_str();

                // If we have 2+ operands, get the second operand as a number
                if (instruction.operands_size() > 1)
                {
                    auto expressionCount = (int)instruction.operands(1).float_value();

                    // And get that many expressions off the stack and build the
                    // collection of substitutions (in reverse order)
                    TArray<FString> substitutions;

                    for (int expressionIndex = expressionCount - 1; expressionIndex >= 0; expressionIndex--)
                    {
                        auto top = state.PopValue();
                        FString topAsString = top.ConvertToString();
                        substitutions.Add(topAsString);
                    }

                    Algo::Reverse(substitutions);

                    line.Substitutions = substitutions;
                }

                // Mark that we're currently delivering content
                SetCurrentExecutionState(DELIVERING_CONTENT);

                // Call the line handler
                OnLine.Broadcast(line);

                // If we're still marked as delivering content, then the line
                // handler didn't call Continue, so we'll wait here
                if (GetCurrentExecutionState() == DELIVERING_CONTENT)
                {
                    SetCurrentExecutionState(WAITING_FOR_CONTINUE);
                }

                break;
            }
        case Yarn::Instruction_OpCode_RUN_COMMAND:
            {
                std::string commandText = instruction.operands(0).string_value();

                // If we have 2+ operands, get the second operand as a number
                if (instruction.operands_size() > 1)
                {
                    auto expressionCount = (int)instruction.operands(1).float_value();

                    // And get that many expressions off the stack and build the
                    // collection of substitutions (in reverse order)
                    TArray<FString> substitutions;

                    for (int expressionIndex = expressionCount - 1; expressionIndex >= 0; expressionIndex--)
                    {
                        auto top = state.PopValue();
                        const std::string topAsString = TCHAR_TO_UTF8(*top.ConvertToString());

                        // FString placeholder("\\{" << expressionIndex << "\\}");
                        commandText = std::regex_replace(commandText, std::regex("\\{" + std::to_string(expressionIndex) + "\\}"), topAsString);
                    }
                }

                SetCurrentExecutionState(DELIVERING_CONTENT);

                auto command = Command();
                command.Text = commandText.c_str();

                OnCommand.Broadcast(command);

                if (GetCurrentExecutionState() == DELIVERING_CONTENT)
                {
                    // The client didn't call Continue, so we'll wait here.
                    SetCurrentExecutionState(WAITING_FOR_CONTINUE);
                }

                break;
            }
        case Yarn::Instruction_OpCode_STOP:
            {
                OnNodeComplete.Broadcast(currentNode.name().c_str());
                OnDialogueComplete.Broadcast();
                SetCurrentExecutionState(STOPPED);
                break;
            }
        case Yarn::Instruction_OpCode_PUSH_BOOL:
            {
                bool value = instruction.operands(0).bool_value();
                state.PushValue(value);
                break;
            }
        case Yarn::Instruction_OpCode_PUSH_FLOAT:
            {
                float value = instruction.operands(0).float_value();
                state.PushValue(value);
                break;
            }
        case Yarn::Instruction_OpCode_PUSH_STRING:
            {
                FString value = instruction.operands(0).string_value().c_str();
                state.PushValue(value);
                break;
            }
        case Yarn::Instruction_OpCode_JUMP_IF_FALSE:
            {
                bool topOfStack = state.PeekValue().GetValue<bool>();
                if (topOfStack == false)
                {
                    auto label = instruction.operands(0).string_value().c_str();
                    state.programCounter = FindInstructionPointForLabel(label) - 1;
                }
                break;
            }
        case Yarn::Instruction_OpCode_JUMP_TO:
            {
                auto label = instruction.operands(0).string_value().c_str();
                state.programCounter = FindInstructionPointForLabel(label) - 1;
                break;
            }
        case Yarn::Instruction_OpCode_JUMP:
            {
                // Jumps to a label whose name is on the stack.
                FString jumpDestination = state.PeekValue().GetValue<FString>();
                state.programCounter = FindInstructionPointForLabel(jumpDestination) - 1;
                break;
            }
        case Yarn::Instruction_OpCode_ADD_OPTION:
            {
                Line line = Line();

                line.LineID = instruction.operands(0).string_value().c_str();
                FString destination = instruction.operands(1).string_value().c_str();

                if (instruction.operands_size() > 2)
                {
                    // The third operand is the number of substitutions present in the
                    // line.

                    auto expressionCount = (int)instruction.operands(2).float_value();

                    // Get that many expressions off the stack and build the collection
                    // of substitutions (in reverse order)
                    TArray<FString> substitutions;

                    for (int expressionIndex = expressionCount - 1; expressionIndex >= 0; expressionIndex--)
                    {
                        auto top = state.PopValue();
                        FString topAsString = top.ConvertToString();
                        substitutions.Add(topAsString);
                    }

                    Algo::Reverse(substitutions);

                    line.Substitutions = substitutions;
                }

                // Indicates whether the VM believes that the option should be shown to
                // the user, based on any conditions that were attached to the option.
                bool lineConditionPassed = true;

                if (instruction.operands_size() > 3)
                {
                    // The fourth operand is a bool that indicates whether this option
                    // had a condition or not. If it does, then a bool value will exist
                    // on the stack indiciating whether the condition passed or not. We
                    // pass that information to the game.

                    bool hasLineCondition = instruction.operands(3).bool_value();

                    if (hasLineCondition)
                    {
                        // This option has a condition. Get it from the stack.
                        lineConditionPassed = state.PopValue().GetValue<bool>();
                    }
                }

                state.AddOption(line, destination, lineConditionPassed);
                break;
            }
        case Yarn::Instruction_OpCode_SHOW_OPTIONS:
            {
                // Show all accumulated options to the game.

                // If we have no options to show, immediately stop.

                if (state.currentOptions.IsEmpty())
                {
                    SetCurrentExecutionState(STOPPED);
                    OnDialogueComplete.Broadcast();
                    break;
                }

                // Present the list of options to the user and let them pick
                auto optionSet = OptionSet();

                optionSet.Options = state.currentOptions;

                // We can't continue until our client tell us which
                // option to pick
                SetCurrentExecutionState(WAITING_ON_OPTION_SELECTION);

                // Pass the options set to the client, as well as a
                // delegate for them to call when the user has made
                // a selection
                OnOptions.Broadcast(optionSet);

                if (GetCurrentExecutionState() == WAITING_FOR_CONTINUE)
                {
                    // we are no longer waiting on an option
                    // selection - the options handler must have
                    // called SetSelectedOption! Continue running
                    // immediately.
                    SetCurrentExecutionState(RUNNING);
                }

                break;
            }
        case Yarn::Instruction_OpCode_PUSH_NULL:
            {
                // Push a null value. This is not a valid instruction as of Yarn Spinner
                // 2.0.
                YS_ERR("PUSH_NULL is not a valid instruction in Yarn Spinner 2.0+");
                return false;
                break;
            }
        case Yarn::Instruction_OpCode_POP:
            {
                // Remove a value from the top of the stack and discard it.
                state.PopValue();
                break;
            }
        case Yarn::Instruction_OpCode_CALL_FUNC:
            {
                // Call a named function, with parameters found on the stack, and push
                // the resulting value onto the stack.
                const FString functionName = instruction.operands(0).string_value().c_str();

                auto actualParamCount = (int)state.PopValue().GetValue<double>();
                if (!OnCheckFunctionExist.Execute(functionName))
                {
                    YS_ERR("Unknown function '%s'", *functionName);
                    return false;
                }

                // auto expectedParamCount = library.GetExpectedParameterCount(functionName);
                auto expectedParamCount = OnGetFunctionParamNum.Execute(functionName);

                if (expectedParamCount >= 0 && expectedParamCount != actualParamCount)
                {
                    YS_ERR("Function '%s' expects %i parameters, but %i were provided", *functionName, expectedParamCount, actualParamCount);
                    return false;
                }

                TArray<FValue> parameters;

                for (int param = actualParamCount - 1; param >= 0; param--)
                {
                    auto value = state.PopValue();
                    parameters.Add(value);
                }
                Algo::Reverse(parameters);

                auto result = OnCallFunction.Execute(functionName, parameters);
                state.PushValue(result);

                // if (library.HasFunction<FString>(functionName))
                // {
                //     auto function = library.GetFunction<FString>(functionName);
                //     auto result = function.Function(parameters);
                //     state.PushValue(result);
                // }
                // else if (library.HasFunction<float>(functionName))
                // {
                //     auto function = library.GetFunction<float>(functionName);
                //     auto result = function.Function(parameters);
                //     state.PushValue(result);
                // }
                // else if (library.HasFunction<bool>(functionName))
                // {
                //     auto function = library.GetFunction<bool>(functionName);
                //     auto result = function.Function(parameters);
                //     state.PushValue(result);
                // }
                // else
                // {
                //     YS_ERR("Unknown function %s", *functionName);
                //     return false;
                // }

                YS_LOG("Function call returned \"%s\" (type: %d)", *state.PeekValue().ConvertToString(), state.PeekValue().GetType());

                break;
            }
        case Yarn::Instruction_OpCode_PUSH_VARIABLE:
            {
                // Get the contents of a variable, and push that onto the stack.
                const FString variableName = instruction.operands(0).string_value().c_str();

                if (variableStorage.HasValue(variableName))
                {
                    // We found a value for this variable in the storage.
                    FValue v = variableStorage.GetValue(variableName);
                    state.PushValue(v);
                }
                else if (program.initial_values().count(TCHAR_TO_UTF8(*variableName)) > 0)
                {
                    // We don't have a value for this. The initial value may be found in
                    // the program. (If it's not, then the variable's value is
                    // undefined, which isn't allowed.)
                    auto operand = program.initial_values().at(TCHAR_TO_UTF8(*variableName));
                    switch (operand.value_case())
                    {
                    case Yarn::Operand::ValueCase::kBoolValue:
                        state.PushValue(operand.bool_value());
                        break;
                    case Yarn::Operand::ValueCase::kStringValue:
                        state.PushValue(operand.string_value().c_str());
                        break;
                    case Yarn::Operand::ValueCase::kFloatValue:
                        state.PushValue(operand.float_value());
                        break;
                    default:
                        YS_ERR("Unknown initial value type %i for variable %s", operand.value_case(), *variableName);
                        return false;
                    }
                }
                else
                {
                    // We didn't find a value for this variable in storage or in the
                    // program's initial values. This is an error - the variable must not
                    // have been defined.
                    YS_ERR("Undefined variable %s", *variableName);
                    return false;
                }
                break;
            }
        case Yarn::Instruction_OpCode_STORE_VARIABLE:
            {
                // Store the top value on the stack in a variable.
                const FValue topValue = state.PeekValue();
                const FString destinationVariableName = instruction.operands(0).string_value().c_str();

                YS_LOG("Set %ss to %s", *destinationVariableName, *topValue.ConvertToString());

                switch (topValue.GetType())
                {
                case FValue::EValueType::String:
                    variableStorage.SetValue(destinationVariableName, topValue.GetValue<FString>());
                    break;
                case FValue::EValueType::Number:
                    variableStorage.SetValue(destinationVariableName, static_cast<float>(topValue.GetValue<double>()));
                    break;
                case FValue::EValueType::Bool:
                    variableStorage.SetValue(destinationVariableName, topValue.GetValue<bool>());
                    break;
                default:
                    YS_ERR("Invalid Yarn value type %i for variable %s", topValue.GetType(), *destinationVariableName);
                    return false;
                }
                break;
            }
        case Yarn::Instruction_OpCode_RUN_NODE:
            {
                // Pop a string from the stack, and jump to a node with that name.
                const FString nodeName = state.PopValue().GetValue<FString>();

                OnNodeComplete.Broadcast(currentNode.name().c_str());

                SetNode(nodeName);

                // Decrement program counter here, because it will be incremented when
                // this function returns, and would mean skipping the first instruction
                state.programCounter -= 1;

                break;
            }
        default:
            YS_LOG("Unhandled instruction type %i", instruction.opcode());
            return false;
            break;
        }

        if (GetCurrentExecutionState() != ERROR)
        {
            return true;
        }
        else
        {
            return false;
        }
    }


    int VirtualMachine::FindInstructionPointForLabel(const FString& Label)
    {
        if (currentNode.labels().count(TCHAR_TO_UTF8(*Label)) == 0)
        {
            YS_ERR("Unknown label %s in node %s", *Label, currentNode.name().c_str());
            SetCurrentExecutionState(ERROR);
            return -1;
        }
        return currentNode.labels().at(TCHAR_TO_UTF8(*Label));
    }


    bool VirtualMachine::CheckCanContinue()
    {
        if (executionState == WAITING_ON_OPTION_SELECTION)
        {
            YS_ERR("Cannot continue running dialogue. Still waiting on option selection.");
            return false;
        }

        if (!OnLine.IsBound())
        {
            YS_ERR("Cannot continue dialogue: OnLine handler has not been set.");
            return false;
        }
        if (!OnOptions.IsBound())
        {
            YS_ERR("Cannot continue dialogue: OnOptions handler has not been set.");
            return false;
        }
        if (!OnCommand.IsBound())
        {
            YS_ERR("Cannot continue dialogue: OnCommand handler has not been set.");
            return false;
        }
        if (!OnNodeComplete.IsBound())
        {
            YS_ERR("Cannot continue dialogue: OnNodeComplete handler has not been set.");
            return false;
        }
        if (!OnDialogueComplete.IsBound())
        {
            YS_ERR("Cannot continue dialogue: OnDialogueComplete handler has not been set.");
            return false;
        }
        return true;
    }


    void VirtualMachine::SetSelectedOption(int selectedOptionIndex)
    {
        if (GetCurrentExecutionState() != WAITING_ON_OPTION_SELECTION)
        {
            YS_ERR("SetSelectedOption was called, but Dialogue wasn't waiting for a selection. This method should only be called after the Dialogue is waiting for the user to select an option.");
            SetCurrentExecutionState(VirtualMachine::ERROR);
            return;
        }

        if (selectedOptionIndex < 0 || selectedOptionIndex >= state.currentOptions.Num())
        {
            YS_LOG("SetSelectedOption was called with an invalid option index");
        }

        auto destinationNode = state.currentOptions[selectedOptionIndex].DestinationNode;
        state.PushValue(destinationNode);

        state.currentOptions.Empty();

        SetCurrentExecutionState(WAITING_FOR_CONTINUE);
    }


    FString VirtualMachine::ExpandSubstitutions(const FString& TemplateString, const TArray<FString>& Substitutions)
    {
        int i = 0;

        std::string Output = TCHAR_TO_UTF8(*TemplateString);
        for (const FString& Sub : Substitutions)
        {
            Output = std::regex_replace(Output, std::regex("\\{" + std::to_string(i) + "\\}"), TCHAR_TO_UTF8(*Sub));
            i++;
        }

        return FString(Output.c_str());
    }
}

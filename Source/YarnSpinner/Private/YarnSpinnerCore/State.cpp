#include "YarnSpinnerCore/State.h"

namespace Yarn
{

    void State::AddOption(const Line &Line, const FString& Destination, const bool bEnabled)
    {
        auto option = Option();

        option.Line = Line;
        option.DestinationNode = Destination;
        option.IsAvailable = bEnabled;
        option.ID = currentOptions.Num();

        currentOptions.Add(option);
    }

    void State::ClearOptions()
    {
        currentOptions.Empty();
    }

    TArray<Option> State::GetCurrentOptions()
    {
        return currentOptions;
    }

    void State::PushValue(const FString& Str)
    {
        stack.Emplace(FValue(Str));
    }

    void State::PushValue(const char* String)
    {
        stack.Add(FValue(String));
    }

    void State::PushValue(const double Number)
    {
        stack.Add(FValue(Number));
    }

    void State::PushValue(const float Number)
    {
        stack.Add(FValue(Number));
    }

    void State::PushValue(const int Number)
    {
        stack.Add(FValue(Number));
    }

    void State::PushValue(const bool bVal)
    {
        stack.Add(FValue(bVal));
    }

    void State::PushValue(const FValue& Value)
    {
        stack.Add(Value);
    }

    FValue State::PopValue()
    {
        return stack.Pop();
    }

    FValue& State::PeekValue()
    {
        return stack.Last();
    }

    void State::ClearStack()
    {
        stack.Empty();
    }

}
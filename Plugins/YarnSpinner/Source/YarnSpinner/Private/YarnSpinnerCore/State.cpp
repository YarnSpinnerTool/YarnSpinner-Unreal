#include "State.h"

namespace Yarn
{

    void State::AddOption(Line &line, const char *destination, bool enabled)
    {
        auto option = Option();

        option.Line = line;
        option.DestinationNode = destination;
        option.IsAvailable = enabled;
        option.ID = currentOptions.size();

        currentOptions.push_back(option);
    }

    void State::ClearOptions()
    {
        currentOptions.clear();
    }

    const std::vector<Option> State::GetCurrentOptions()
    {
        return currentOptions;
    }

    void State::PushValue(std::string string)
    {
        stack.push_back(Value(string));
    }

    void State::PushValue(const char *string)
    {
        stack.push_back(Value(string));
    }

    void State::PushValue(double number)
    {
        stack.push_back(Value(number));
    }

    void State::PushValue(float number)
    {
        stack.push_back(Value(number));
    }

    void State::PushValue(int number)
    {
        stack.push_back(Value(number));
    }

    void State::PushValue(bool boolean)
    {
        stack.push_back(Value(boolean));
    }

    void State::PushValue(Value value)
    {
        stack.push_back(value);
    }

    Value State::PopValue()
    {
        auto last = stack.back();
        stack.pop_back();
        return last;
    }

    Value State::PeekValue()
    {
        return stack.back();
    }

    void State::ClearStack()
    {
    }

}
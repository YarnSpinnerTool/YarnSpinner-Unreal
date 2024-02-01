#pragma once

#include <string>
#include <stack>
#include <vector>

#include "YarnSpinnerCore/Common.h"
#include "Value.h"

namespace Yarn
{
    class YARNSPINNER_API State
    {
    public:
        std::vector<FValue> stack;
        std::vector<Option> currentOptions;

        std::string currentNodeName;

        int programCounter = 0;

        void AddOption(Line &line, const char *destination, bool enabled);
        void ClearOptions();
        const std::vector<Option> GetCurrentOptions();

        void PushValue(std::string string);
        void PushValue(const char *string);
        void PushValue(double number);
        void PushValue(float number);
        void PushValue(int number);
        void PushValue(bool boolean);

        void PushValue(FValue value);

        FValue PopValue();
        FValue PeekValue();

        void ClearStack();
    };
}

#pragma once

#include <string>
#include <vector>

#include "Value.h"
#include "YarnSpinnerCore/Common.h"

namespace Yarn
{
    class YARNSPINNER_API State
    {
    public:
        TArray<FValue> stack;
        TArray<Option> currentOptions;

        FString currentNodeName;

        int programCounter = 0;

        void AddOption(const Line &Line, const FString& Destination, bool bEnabled);
        void ClearOptions();
        TArray<Option> GetCurrentOptions();

        void PushValue(const FString& Str);
        void PushValue(const char *String);
        void PushValue(double Number);
        void PushValue(float Number);
        void PushValue(int Number);
        void PushValue(bool bVal);

        void PushValue(const FValue& Value);

        FValue PopValue();
        FValue& PeekValue();

        void ClearStack();
    };
}

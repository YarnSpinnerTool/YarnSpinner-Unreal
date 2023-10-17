#pragma once

#include "CoreMinimal.h"
#include "YarnSpinnerCore/Value.h"


class FYarnValueHelpers
{
public:
    static FString GetTypeString(const Yarn::Value& Value)
    {
        return Value.GetType() == Yarn::Value::STRING ? "string" : (Value.GetType() == Yarn::Value::NUMBER ? "number" : "boolean");
    }
};



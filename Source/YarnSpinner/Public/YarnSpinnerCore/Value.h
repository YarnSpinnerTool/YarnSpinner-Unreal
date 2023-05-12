#pragma once

#include <string>
#include "YarnSpinnerCore/Common.h"
#include <cmath>

namespace Yarn
{
    class YARNSPINNER_API Value
    {

    public:
        enum ValueType
        {
            STRING,
            NUMBER,
            BOOL
        };

        ValueType type;

        std::string stringValue;
        double number;
        bool boolean;

        Value(const char *string) : type(ValueType::STRING), stringValue(std::string(string)) {}

        Value(const std::string &string) : type(ValueType::STRING), stringValue(string) {}

        Value(float number) : type(ValueType::NUMBER), number(number) {}
        Value(double number) : type(ValueType::NUMBER), number(number) {}
        Value(int number) : type(ValueType::NUMBER), number(number) {}

        Value(bool boolean) : type(ValueType::BOOL), boolean(boolean) {}

        ValueType GetType()
        {
            return this->type;
        }

        const std::string GetStringValue()
        {
            if (this->type == STRING)
            {
                return this->stringValue;
            }
            else
            {
                return "";
            }
        }

        float GetNumberValue()
        {
            if (this->type == NUMBER)
            {
                return this->number;
            }
            else
            {
                return 0;
            }
        }

        bool GetBooleanValue()
        {
            if (this->type == BOOL)
            {
                return this->boolean;
            }
            else
            {
                return false;
            }
        }

        const std::string ConvertToString()
        {
            switch (type)
            {
            case STRING:
                return stringValue;
            case BOOL:
                return boolean ? "True" : "False";
            case NUMBER:
                if (trunc(number) == number)
                {
                    return std::to_string((int)number);
                }
                else
                {
                    return std::to_string(number);
                }
            default:
                return "<unknown>";
            }
        }
    };
}

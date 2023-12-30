#pragma once

#include <string>
#include <cmath>

namespace Yarn
{
    class YARNSPINNER_API FValue
    {
    public:
        enum EValueType
        {
            String,
            Number,
            Bool
        };

        // Default constructor
        FValue() : Data(TInPlaceType<double>(), 0) {}
        
        // Value constructors
        explicit FValue(const char* const Str) : Data(TInPlaceType<FString>(), Str) {}
        explicit FValue(const std::string& Str) : Data(TInPlaceType<FString>(), Str.c_str()) {}
        explicit FValue(const FString& Str) : Data(TInPlaceType<FString>(), Str) {}
        explicit FValue(float Num) : Data(TInPlaceType<double>(), MoveTemp(Num)) {}
        explicit FValue(double Num) : Data(TInPlaceType<double>(), MoveTemp(Num)) {}
        explicit FValue(int Num) : Data(TInPlaceType<double>(), MoveTemp(Num)) {}
        explicit FValue(bool Val) : Data(TInPlaceType<bool>(), MoveTemp(Val)) {}
        
        // Copy & Move constructors
        FValue(const FValue& Other) : Data(Other.Data) {}
        FValue(FValue&& Other) noexcept : Data(MoveTemp(Other.Data)) {}

        // Assignment
        FValue& operator=(const FValue& Other)
        {
            if (this == &Other)
                return *this;
            Data = Other.Data;
            return *this;
        }

        FValue& operator=(const FString& Str)
        {
            Data.Set<FString>(Str);
            return *this;
        }
        
        FValue& operator=(const double& Number)
        {
            Data.Set<double>(Number);
            return *this;
        }

        FValue& operator=(const bool& Value)
        {
            Data.Set<bool>(Value);
            return *this;
        }

        FValue& operator=(FValue&& Other) noexcept
        {
            if (this == &Other)
                return *this;
            Data = MoveTemp(Other.Data);
            return *this;
        }

        FValue& operator=(FString&& Str) noexcept
        {
            Data.Set<FString>(MoveTemp(Str));
            return *this;
        }

        FValue& operator=(double&& Number) noexcept
        {
            Data.Set<double>(MoveTemp(Number));
            return *this;
        }

        FValue& operator=(bool&& Value) noexcept
        {
            Data.Set<bool>(MoveTemp(Value));
            return *this;
        }

        // Value accessors
        template<typename T>
        T GetValue() const
        {
            unimplemented();
        }

        UE_NODISCARD EValueType GetType() const
        {
            if (Data.IsType<FString>())
            {
                return String;
            }

            if (Data.IsType<bool>())
            {
                return Bool;
            }

            return Number;
        }

        UE_NODISCARD FString ConvertToString() const
        {
            switch (GetType())
            {
            case String:
                return Data.Get<FString>();
            case Number:
                {
                    const double& NumberValue = Data.Get<double>();
                    return trunc(NumberValue) == NumberValue ?
                        FString::FromInt(static_cast<int>(NumberValue)) :
                    FString::SanitizeFloat(NumberValue);
                }
            case Bool:
                return Data.Get<bool>() ? "True" : "False";
            default:
                return "<unknown>";
            }
        }
        
        UE_NODISCARD double ConvertToNumber() const
        {
            switch (GetType())
            {
            case String:
                return FCString::Atod(*Data.Get<FString>());
            case Bool:
                return Data.Get<bool>() ? 1 : 0;
            case Number:
                return Data.Get<double>();
            default:
                return 0;
            }
        }
        
    protected:
        TVariant<FString, double, bool> Data;
    };

    template <>
    UE_NODISCARD FORCEINLINE double FValue::GetValue<double>() const
    {
        return Data.IsType<double>() ?
            Data.Get<double>() :
            0;
    }

    template <>
    UE_NODISCARD FORCEINLINE bool FValue::GetValue<bool>() const
    {
        return Data.IsType<bool>() ?
            Data.Get<bool>() :
            false;
    }

    template <>
    UE_NODISCARD FORCEINLINE FString FValue::GetValue<FString>() const
    {
        return Data.IsType<FString>() ?
            Data.Get<FString>() :
            FString();
    }
}

// String value conversion
YARNSPINNER_API UE_NODISCARD FORCEINLINE FString LexToString(const Yarn::FValue::EValueType ValueType)
{
    switch (ValueType)
    {
    case Yarn::FValue::EValueType::String:
        return "string";
    case Yarn::FValue::EValueType::Bool:
        return "boolean";
    case Yarn::FValue::EValueType::Number:
        return "number";
    }

    return "number";
}

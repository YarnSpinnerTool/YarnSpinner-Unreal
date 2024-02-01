#include "YarnSpinnerCore/Library.h"

#if defined(_MSC_VER)
	#pragma warning (disable:4800) // 'type' : forcing value to bool 'true' or 'false' (performance warning)
#endif

namespace Yarn
{

    // template <typename R, typename... Types>
    // constexpr size_t GetArgumentCount(R (*f)(Types...))
    // {
    //     return sizeof...(Types);
    // }

    template <typename T>
    FunctionInfo<T>::FunctionInfo() : ExpectedParameterCount(0), Function()
    {
    }
    template <typename T>
    FunctionInfo<T>::FunctionInfo(int paramCount, T (*f)(std::vector<FValue>))
    {
        Function = f;
        ExpectedParameterCount = paramCount;
    }

    Library::Library(ILogger &logger) : logger(logger)
    {
        LoadStandardLibrary();
    }

    // Helper type used for statically asserting that a given type T is invalid
    // https://stackoverflow.com/questions/6947805/block-non-specialized-template-c
    template <typename T>
    struct dependent_false
    {
        enum
        {
            value = false
        };
    };

    // Get function

    template <typename T>
    FunctionInfo<T> Library::GetFunction(std::string name)
    {
        UNUSED(name);
        static_assert(dependent_false<T>::value, "Invalid return type for function");
    }

    template <>
    FunctionInfo<std::string> Library::GetFunction<std::string>(std::string name)
    {
        return stringFunctions[name];
    }

    template <>
    FunctionInfo<float> Library::GetFunction<float>(std::string name)
    {
        return numberFunctions[name];
    }

    template <>
    FunctionInfo<bool> Library::GetFunction<bool>(std::string name)
    {
        return boolFunctions[name];
    }

    template <typename T>
    FunctionInfo<T> &Library::GetFunctionImpl(std::map<std::string, FunctionInfo<T>> &source, std::string name)
    {
        if (source.count(name) == 0)
        {
            logger.Log(string_format("Can't get implementation for unknown function '%s'", UTF8_TO_TCHAR(name.c_str())));
            return FunctionInfo<T>();
        }
        return source[name];
    }

    // Add Function

    // template <typename T>
    // void Library::AddFunction (std::string name, T(*f)(std::vector<Value>), int parameterCount) {
    //     // A helper function that can take any function pointer (including lambdas),
    //     // converts them to a YarnFunction<T>, and then forwards that to the regular
    //     // YarnFunction<T> method
    //     YarnFunction<T> yarnFunction = YarnFunction<T>(f);
    //     AddFunction(name, yarnFunction, parameterCount);
    // }

    template <typename T>
    void Library::AddFunction(std::string name, YarnFunction<T> function, int parameterCount)
    {
        UNUSED(name);
        UNUSED(function);
        UNUSED(parameterCount);
        static_assert(dependent_false<T>::value, "Invalid return type for function");
    }

    template <>
    void Library::AddFunction<std::string>(std::string name, YarnFunction<std::string> function, int parameterCount)
    {
        AddFunctionImpl(stringFunctions, name, function, parameterCount);
    }

    template <>
    void Library::AddFunction<float>(std::string name, YarnFunction<float> function, int parameterCount)
    {
        AddFunctionImpl(numberFunctions, name, function, parameterCount);
    }

    template <>
    void Library::AddFunction<bool>(std::string name, YarnFunction<bool> function, int parameterCount)
    {
        AddFunctionImpl(boolFunctions, name, function, parameterCount);
    }

    template <typename T>
    void Library::AddFunctionImpl(std::map<std::string, FunctionInfo<T>> &source, std::string name, YarnFunction<T> func, int parameterCount)
    {
        // Report an error if this function is already defined across any of our
        // function tables
        if (
            stringFunctions.count(name) > 0 ||
            numberFunctions.count(name) > 0 ||
            boolFunctions.count(name) > 0 ||
            source.count(name) > 0) // strictly unnecessary but might help catch future bugs

        {
            logger.Log(string_format("Function %s is already defined", UTF8_TO_TCHAR(name.c_str())));
            return;
        }

        // Register information about this function!
        FunctionInfo<T> info;
        info.ExpectedParameterCount = parameterCount;
        info.Function = func;
        source[name] = info;
    }

    // Remove function

    template <typename T>
    void Library::RemoveFunction(std::string name)
    {
        UNUSED(name);
        static_assert(dependent_false<T>::value, "Invalid return type for function");
    }

    template <>
    void Library::RemoveFunction<std::string>(std::string name)
    {
        RemoveFunctionImpl(stringFunctions, name);
    }

    template <>
    void Library::RemoveFunction<float>(std::string name)
    {
        RemoveFunctionImpl(stringFunctions, name);
    }

    template <>
    void Library::RemoveFunction<bool>(std::string name)
    {
        RemoveFunctionImpl(stringFunctions, name);
    }

    template <typename T>
    void Library::RemoveFunctionImpl(std::map<std::string, FunctionInfo<T>> &source, std::string name)
    {
        if (source.count(name) > 0)
        {
            source.erase(name);
        }
    }

    // Has Function
    template <typename T>
    bool Library::HasFunction(std::string name)
    {
        UNUSED(name);
        static_assert(dependent_false<T>::value, "Invalid return type for function");
    }

    template <>
    bool Library::HasFunction<std::string>(std::string name)
    {
        return HasFunctionImpl<std::string>(stringFunctions, name);
    }

    template <>
    bool Library::HasFunction<float>(std::string name)
    {
        return HasFunctionImpl<float>(numberFunctions, name);
    }

    template <>
    bool Library::HasFunction<bool>(std::string name)
    {
        return HasFunctionImpl<bool>(boolFunctions, name);
    }

    template <typename T>
    bool Library::HasFunctionImpl(std::map<std::string, FunctionInfo<T>> &source, std::string name)
    {
        return source.count(name) > 0;
    }

    // Remove all functions
    void Library::RemoveAllFunctions()
    {
        stringFunctions.clear();
        numberFunctions.clear();
        boolFunctions.clear();
    }

    // Get parameter count
    int Library::GetExpectedParameterCount(std::string name)
    {
        if (HasFunction<std::string>(name))
        {
            return GetFunction<std::string>(name).ExpectedParameterCount;
        }
        else if (HasFunction<float>(name))
        {
            return GetFunction<float>(name).ExpectedParameterCount;
        }
        else if (HasFunction<bool>(name))
        {
            return GetFunction<bool>(name).ExpectedParameterCount;
        }
        else
        {
            return -1;
        }
    }

    // Load standard library
    void Library::LoadStandardLibrary()
    {
        // Number methods
        AddFunction<bool>(
            "Number.EqualTo",
            [](std::vector<FValue> values)
            {
                return values.at(0).GetValue<double>() == values.at(1).GetValue<double>();
            },
            2);

        AddFunction<bool>(
            "Number.NotEqualTo",
            [](std::vector<FValue> values)
            {
                return values.at(0).GetValue<double>() != values.at(1).GetValue<double>();
            },
            2);

        AddFunction<float>(
            "Number.Add",
            [](std::vector<FValue> values)
            {
                return values.at(0).GetValue<double>() + values.at(1).GetValue<double>();
            },
            2);

        AddFunction<float>(
            "Number.Minus",
            [](std::vector<FValue> values)
            {
                return values.at(0).GetValue<double>() - values.at(1).GetValue<double>();
            },
            2);

        AddFunction<float>(
            "Number.Divide",
            [](std::vector<FValue> values)
            {
                return values.at(0).GetValue<double>() / values.at(1).GetValue<double>();
            },
            2);

        AddFunction<float>(
            "Number.Multiply",
            [](std::vector<FValue> values)
            {
                return values.at(0).GetValue<double>() * values.at(1).GetValue<double>();
            },
            2);

        AddFunction<float>(
            "Number.Modulo",
            [](std::vector<FValue> values)
            {
                return (int)(values.at(0).GetValue<double>()) % (int)(values.at(1).GetValue<double>());
            },
            2);

        AddFunction<float>(
            "Number.UnaryMinus",
            [](std::vector<FValue> values)
            {
                return -(values.at(0).GetValue<double>());
            },
            1);

        AddFunction<bool>(
            "Number.GreaterThan",
            [](std::vector<FValue> values)
            {
                return values.at(0).GetValue<double>() > values.at(1).GetValue<double>();
            },
            2);

        AddFunction<bool>(
            "Number.GreaterThanOrEqualTo",
            [](std::vector<FValue> values)
            {
                return values.at(0).GetValue<double>() >= values.at(1).GetValue<double>();
            },
            2);

        AddFunction<bool>(
            "Number.LessThan",
            [](std::vector<FValue> values)
            {
                return values.at(0).GetValue<double>() < values.at(1).GetValue<double>();
            },
            2);

        AddFunction<bool>(
            "Number.LessThanOrEqualTo",
            [](std::vector<FValue> values)
            {
                return values.at(0).GetValue<double>() <= values.at(1).GetValue<double>();
            },
            2);

        // Boolean methods
        AddFunction<bool>(
            "Bool.EqualTo", [](std::vector<FValue> values)
            { return values.at(0).GetValue<bool>() == values.at(1).GetValue<bool>(); },
            2);

        AddFunction<bool>(
            "Bool.NotEqualTo", [](std::vector<FValue> values)
            { return values.at(0).GetValue<bool>() != values.at(1).GetValue<bool>(); },
            2);

        AddFunction<bool>(
            "Bool.And", [](std::vector<FValue> values)
            { return values.at(0).GetValue<bool>() && values.at(1).GetValue<bool>(); },
            2);

        AddFunction<bool>(
            "Bool.Or", [](std::vector<FValue> values)
            { return values.at(0).GetValue<bool>() || values.at(1).GetValue<bool>(); },
            2);

        AddFunction<bool>(
            "Bool.Xor", [](std::vector<FValue> values)
            { return values.at(0).GetValue<bool>() ^ values.at(1).GetValue<bool>(); },
            2);

        AddFunction<bool>(
            "Bool.Not", [](std::vector<FValue> values)
            { return !values.at(0).GetValue<bool>(); },
            1);

        // String functions

        AddFunction<bool>(
            "String.EqualTo", [](std::vector<FValue> values)
            { return values.at(0).GetValue<FString>() == values.at(1).GetValue<FString>(); },
            2);

        AddFunction<bool>(
            "String.NotEqualTo", [](std::vector<FValue> values)
            { return values.at(0).GetValue<FString>() != values.at(1).GetValue<FString>(); },
            2);

        AddFunction<std::string>(
            "String.Add", [](std::vector<FValue> values)
            { return TCHAR_TO_UTF8(*(values.at(0).GetValue<FString>() + values.at(1).GetValue<FString>())); },
            2);
    }

    std::string Library::GenerateUniqueVisitedVariableForNode(std::string nodeName)
    {
        return "$Yarn.Internal.Visiting." + nodeName;
    }
}

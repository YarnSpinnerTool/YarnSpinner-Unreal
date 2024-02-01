#include "YarnSpinnerCore/Library.h"

#include "Misc/YSLogging.h"
#include "YarnSpinnerCore/Common.h"

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
    FunctionInfo<T>::FunctionInfo(const int ParamCount, T (*F)(const TArray<FValue>&))
    {
        Function = F;
        ExpectedParameterCount = ParamCount;
    }

    Library::Library()
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
    FunctionInfo<T> Library::GetFunction(const FString& Name)
    {
        UNUSED(Name);
        static_assert(dependent_false<T>::value, "Invalid return type for function");
    }

    template <>
    FunctionInfo<FString> Library::GetFunction<FString>(const FString& name)
    {
        return stringFunctions[name];
    }

    template <>
    FunctionInfo<float> Library::GetFunction<float>(const FString& name)
    {
        return numberFunctions[name];
    }

    template <>
    FunctionInfo<bool> Library::GetFunction<bool>(const FString& name)
    {
        return boolFunctions[name];
    }

    template <typename T>
    FunctionInfo<T> &Library::GetFunctionImpl(TMap<FString, FunctionInfo<T>> &Source, const FString& Name)
    {
        if (!Source.contains(Name))
        {
            YS_LOG("Can't get implementation for unknown function '%s'", *Name);
            return FunctionInfo<T>();
        }
        return Source[Name];
    }

    // Add Function

    // template <typename T>
    // void Library::AddFunction (FString name, T(*f)(TArray<Value>), int parameterCount) {
    //     // A helper function that can take any function pointer (including lambdas),
    //     // converts them to a YarnFunction<T>, and then forwards that to the regular
    //     // YarnFunction<T> method
    //     YarnFunction<T> yarnFunction = YarnFunction<T>(f);
    //     AddFunction(name, yarnFunction, parameterCount);
    // }

    template <typename T>
    void Library::AddFunction(const FString& name, TYarnFunction<T> function, int parameterCount)
    {
        UNUSED(name);
        UNUSED(function);
        UNUSED(parameterCount);
        static_assert(dependent_false<T>::value, "Invalid return type for function");
    }

    template <>
    void Library::AddFunction<FString>(const FString& name, TYarnFunction<FString> function, int parameterCount)
    {
        AddFunctionImpl(stringFunctions, name, function, parameterCount);
    }

    template <>
    void Library::AddFunction<float>(const FString& name, TYarnFunction<float> function, int parameterCount)
    {
        AddFunctionImpl(numberFunctions, name, function, parameterCount);
    }

    template <>
    void Library::AddFunction<bool>(const FString& name, TYarnFunction<bool> function, int parameterCount)
    {
        AddFunctionImpl(boolFunctions, name, function, parameterCount);
    }

    template <typename T>
    void Library::AddFunctionImpl(TMap<FString, FunctionInfo<T>> &source, const FString& name, TYarnFunction<T> func, int parameterCount)
    {
        // Report an error if this function is already defined across any of our
        // function tables
        if (
            stringFunctions.Contains(name) ||
            numberFunctions.Contains(name) ||
            boolFunctions.Contains(name) ||
            source.Contains(name)) // strictly unnecessary but might help catch future bugs

        {
            YS_LOG("Function %s is already defined", *name);
            return;
        }

        // Register information about this function!
        FunctionInfo<T> info;
        info.ExpectedParameterCount = parameterCount;
        info.Function = func;
        source.Emplace(name, MoveTemp(info));
    }

    // Remove function

    template <typename T>
    void Library::RemoveFunction(const FString& Name)
    {
        UNUSED(Name);
        static_assert(dependent_false<T>::value, "Invalid return type for function");
    }

    template <>
    void Library::RemoveFunction<FString>(const FString& name)
    {
        RemoveFunctionImpl(stringFunctions, name);
    }

    template <>
    void Library::RemoveFunction<float>(const FString& name)
    {
        RemoveFunctionImpl(stringFunctions, name);
    }

    template <>
    void Library::RemoveFunction<bool>(const FString& name)
    {
        RemoveFunctionImpl(stringFunctions, name);
    }

    template <typename T>
    void Library::RemoveFunctionImpl(TMap<FString, FunctionInfo<T>> &source, const FString& name)
    {
        source.Remove(name);
    }

    // Has Function
    template <typename T>
    bool Library::HasFunction(const FString& Name)
    {
        UNUSED(Name);
        static_assert(dependent_false<T>::value, "Invalid return type for function");
    }

    template <>
    bool Library::HasFunction<FString>(const FString& name)
    {
        return HasFunctionImpl<FString>(stringFunctions, name);
    }

    template <>
    bool Library::HasFunction<float>(const FString& name)
    {
        return HasFunctionImpl<float>(numberFunctions, name);
    }

    template <>
    bool Library::HasFunction<bool>(const FString& name)
    {
        return HasFunctionImpl<bool>(boolFunctions, name);
    }

    template <typename T>
    bool Library::HasFunctionImpl(TMap<FString, FunctionInfo<T>> &source, const FString& name)
    {
        return source.Contains(name);
    }

    // Remove all functions
    void Library::RemoveAllFunctions()
    {
        stringFunctions.Empty();
        numberFunctions.Empty();
        boolFunctions.Empty();
    }

    // Get parameter count
    int Library::GetExpectedParameterCount(const FString& Name)
    {
        if (HasFunction<FString>(Name))
        {
            return GetFunction<FString>(Name).ExpectedParameterCount;
        }
        else if (HasFunction<float>(Name))
        {
            return GetFunction<float>(Name).ExpectedParameterCount;
        }
        else if (HasFunction<bool>(Name))
        {
            return GetFunction<bool>(Name).ExpectedParameterCount;
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
        AddFunction(
            "Number.EqualTo",
            TYarnFunction<bool>::CreateLambda(
            [](const TArray<FValue>& Values)
            {
                return Values[0].GetValue<double>() == Values[1].GetValue<double>();
            }),
            2);

        AddFunction(
            "Number.NotEqualTo",
            TYarnFunction<bool>::CreateLambda(
            [](const TArray<FValue>& Values)
            {
                return Values[0].GetValue<double>() != Values[1].GetValue<double>();
            }),
            2);

        AddFunction(
            "Number.Add",
            TYarnFunction<float>::CreateLambda(
            [](const TArray<FValue>& Values)
            {
                return Values[0].GetValue<double>() + Values[1].GetValue<double>();
            }),
            2);

        AddFunction(
            "Number.Minus",
            TYarnFunction<float>::CreateLambda(
            [](const TArray<FValue>& Values)
            {
                return Values[0].GetValue<double>() - Values[1].GetValue<double>();
            }),
            2);

        AddFunction(
            "Number.Divide",
            TYarnFunction<float>::CreateLambda(
            [](const TArray<FValue>& Values)
            {
                return Values[0].GetValue<double>() / Values[1].GetValue<double>();
            }),
            2);

        AddFunction(
            "Number.Multiply",
            TYarnFunction<float>::CreateLambda(
            [](const TArray<FValue>& Values)
            {
                return Values[0].GetValue<double>() * Values[1].GetValue<double>();
            }),
            2);

        AddFunction(
            "Number.Modulo",
            TYarnFunction<float>::CreateLambda(
            [](const TArray<FValue>& Values)
            {
                return static_cast<int>(Values[0].GetValue<double>()) % static_cast<int>(Values[1].GetValue<double>());
            }),
            2);

        AddFunction(
            "Number.UnaryMinus",
            TYarnFunction<float>::CreateLambda(
            [](const TArray<FValue>& Values)
            {
                return -Values[0].GetValue<double>();
            }),
            1);

        AddFunction(
            "Number.GreaterThan",
            TYarnFunction<bool>::CreateLambda(
            [](const TArray<FValue>& Values)
            {
                return Values[0].GetValue<double>() > Values[1].GetValue<double>();
            }),
            2);

        AddFunction(
            "Number.GreaterThanOrEqualTo",
            TYarnFunction<bool>::CreateLambda(
            [](const TArray<FValue>& Values)
            {
                return Values[0].GetValue<double>() >= Values[1].GetValue<double>();
            }),
            2);

        AddFunction(
            "Number.LessThan",
            TYarnFunction<bool>::CreateLambda(
            [](const TArray<FValue>& Values)
            {
                return Values[0].GetValue<double>() < Values[1].GetValue<double>();
            }),
            2);

        AddFunction(
            "Number.LessThanOrEqualTo",
            TYarnFunction<bool>::CreateLambda(
            [](const TArray<FValue>& Values)
            {
                return Values[0].GetValue<double>() <= Values[1].GetValue<double>();
            }),
            2);

        // Boolean methods
        AddFunction(
            "Bool.EqualTo", 
            TYarnFunction<bool>::CreateLambda(
                [](const TArray<FValue>& Values)
                { return Values[0].GetValue<bool>() == Values[1].GetValue<bool>(); }),
            2);

        AddFunction(
            "Bool.NotEqualTo",
            TYarnFunction<bool>::CreateLambda(
                [](const TArray<FValue>& Values)
                { return Values[0].GetValue<bool>() != Values[1].GetValue<bool>(); }),
            2);

        AddFunction(
            "Bool.And", TYarnFunction<bool>::CreateLambda(
                [](const TArray<FValue>& Values)
                { return Values[0].GetValue<bool>() && Values[1].GetValue<bool>(); }),
            2);

        AddFunction(
            "Bool.Or", TYarnFunction<bool>::CreateLambda(
                [](const TArray<FValue>& Values)
                { return Values[0].GetValue<bool>() || Values[1].GetValue<bool>(); }),
            2);

        AddFunction<bool>(
            "Bool.Xor", TYarnFunction<bool>::CreateLambda(
                [](const TArray<FValue>& Values)
                { return Values[0].GetValue<bool>() ^ Values[1].GetValue<bool>(); }),
            2);

        AddFunction(
            "Bool.Not", TYarnFunction<bool>::CreateLambda(
                [](const TArray<FValue>& Values)
                { return !Values[0].GetValue<bool>(); }),
            1);

        // String functions

        AddFunction(
            "String.EqualTo", TYarnFunction<bool>::CreateLambda(
                [](const TArray<FValue>& Values)
                { return Values[0].GetValue<FString>() == Values[1].GetValue<FString>(); }),
            2);

        AddFunction(
            "String.NotEqualTo", TYarnFunction<bool>::CreateLambda(
                [](const TArray<FValue>& Values)
                { return Values[0].GetValue<FString>() != Values[1].GetValue<FString>(); }),
            2);

        AddFunction(
            "String.Add", TYarnFunction<FString>::CreateLambda(
                [](const TArray<FValue>& Values)
                { return Values[0].GetValue<FString>() + Values[1].GetValue<FString>(); }),
            2);
    }

    FString Library::GenerateUniqueVisitedVariableForNode(const FString& NodeName)
    {
        return FString::Printf(TEXT("$Yarn.Internal.Visiting.%s"), *NodeName);
    }
}

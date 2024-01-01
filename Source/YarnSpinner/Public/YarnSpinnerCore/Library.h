#pragma once

#include <string>

#include "Value.h"

namespace Yarn
{
    template <typename T>
    using TYarnFunction = TDelegate<T(const TArray<FValue>&)>;

    template <typename T>
    class YARNSPINNER_API FunctionInfo
    {
    public:
        int ExpectedParameterCount;
        TYarnFunction<T> Function;

        FunctionInfo();
        FunctionInfo(int ParamCount, T (*F)(const TArray<FValue>&));
    };

    class YARNSPINNER_API Library
    {
    private:
        TMap<FString, FunctionInfo<FString>> stringFunctions;
        TMap<FString, FunctionInfo<float>> numberFunctions;
        TMap<FString, FunctionInfo<bool>> boolFunctions;

        template <typename T>
        FunctionInfo<T> &GetFunctionImpl(TMap<FString, FunctionInfo<T>> &Source, const FString& Name);

        template <typename T>
        void AddFunctionImpl(TMap<FString, FunctionInfo<T>> &source, const FString& name, TYarnFunction<T> func, int parameterCount);

        template <typename T>
        void RemoveFunctionImpl(TMap<FString, FunctionInfo<T>> &source, const FString& name);

        template <typename T>
        bool HasFunctionImpl(TMap<FString, FunctionInfo<T>> &source, const FString& name);

    public:
        Library();

        template <typename T>
        FunctionInfo<T> GetFunction(const FString& Name);

        template <typename T>
        void AddFunction(const FString& name, TYarnFunction<T> function, int parameterCount);

        // template <typename T>
        // void AddFunction (const FString& name, T(*f)(TArray<::Value>), int parameterCount);

        template <typename T>
        void RemoveFunction(const FString& Name);

        template <typename T>
        bool HasFunction(const FString& Name);

        void RemoveAllFunctions();

        int GetExpectedParameterCount(const FString& Name);

        void LoadStandardLibrary();

        static FString GenerateUniqueVisitedVariableForNode(const FString& NodeName);
    };

}
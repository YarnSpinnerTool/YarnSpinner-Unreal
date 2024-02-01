#pragma once

#include <map>
#include <functional>
#include <string>

#include "Value.h"

namespace Yarn
{

    template <typename T>
    using YarnFunction = std::function<T(std::vector<FValue>)>;

    template <typename T>
    class YARNSPINNER_API FunctionInfo
    {
    public:
        int ExpectedParameterCount;
        YarnFunction<T> Function;

        FunctionInfo();
        FunctionInfo(int paramCount, T (*f)(std::vector<FValue>));
    };

    class YARNSPINNER_API Library
    {
    private:
        ILogger &logger;

        std::map<std::string, FunctionInfo<std::string>> stringFunctions;
        std::map<std::string, FunctionInfo<float>> numberFunctions;
        std::map<std::string, FunctionInfo<bool>> boolFunctions;

        template <typename T>
        FunctionInfo<T> &GetFunctionImpl(std::map<std::string, FunctionInfo<T>> &source, std::string name);

        template <typename T>
        void AddFunctionImpl(std::map<std::string, FunctionInfo<T>> &source, std::string name, YarnFunction<T> func, int parameterCount);

        template <typename T>
        void RemoveFunctionImpl(std::map<std::string, FunctionInfo<T>> &source, std::string name);

        template <typename T>
        bool HasFunctionImpl(std::map<std::string, FunctionInfo<T>> &source, std::string name);

    public:
        Library(ILogger &logger);

        template <typename T>
        FunctionInfo<T> GetFunction(std::string name);

        template <typename T>
        void AddFunction(std::string name, YarnFunction<T> function, int parameterCount);

        // template <typename T>
        // void AddFunction (std::string name, T(*f)(std::vector<::Value>), int parameterCount);

        template <typename T>
        void RemoveFunction(std::string name);

        template <typename T>
        bool HasFunction(std::string name);

        void RemoveAllFunctions();

        int GetExpectedParameterCount(std::string name);

        void LoadStandardLibrary();

        static std::string GenerateUniqueVisitedVariableForNode(std::string nodeName);
    };

}
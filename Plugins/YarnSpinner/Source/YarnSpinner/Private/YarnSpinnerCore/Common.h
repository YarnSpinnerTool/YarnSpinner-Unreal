#pragma once

#include <string>
#include <vector>
#include <iostream>

#define UNUSED(x) (void)(x)

namespace Yarn
{

    class ILogger
    {
    public:
        enum Type
        {
            ERROR,
            WARNING,
            INFO
        };
        /**
         * @brief Logs a message.
         * 
         * @param message The message to log.
         * @param severity The severity of the log.
         */
        virtual void Log(std::string message, Type severity = Type::INFO) = 0;
    };

    struct Line
    {
        std::string LineID;
        std::vector<std::string> Substitutions;

        friend std::ostream &operator<<(std::ostream &os, const Line &line)
        {
            os << "ID: " << line.LineID;

            int i = 0;
            for (auto sub : line.Substitutions)
            {
                os << ", {" << i << "} :" << sub;
                i++;
            }
            return os;
        }
    };

    struct OptionSet
    {
        std::vector<struct Option> Options;
    };

    struct Option
    {
        Line Line;
        int ID = -1;
        std::string DestinationNode;
        bool IsAvailable = true;
    };

    struct Command
    {
        std::string Text;
    };

    template <typename... Args>
    std::string string_format(const std::string &format, Args... args)
    {
        int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
        if (size_s <= 0)
        {
            throw std::runtime_error("Error during formatting.");
        }
        auto size = static_cast<size_t>(size_s);
        std::unique_ptr<char[]> buf(new char[size]);
        std::snprintf(buf.get(), size, format.c_str(), args...);
        return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
    }
}
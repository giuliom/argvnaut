#pragma once

#include <string>
#include <vector>

namespace ArgvNaut {

enum class ArgumentType 
{
    POSITIONAL,
    SUBCOMMAND, // TODO handle subcommands by running a new instance of ArgvNaut
    FLAG,
    OPTION, // TODO handle lists and maps 
};

enum class OptionType {
    BOOLEAN,
    INTEGER,
    FLOAT,
    STRING,
};

struct PositionalArg {
    std::string name;
    bool required = true;
};

struct SubcommandArg {
    std::string name;
    std::vector<std::string> aliases;
};

struct FlagArg {
    std::string name;
    std::string shortName;
    bool defaultValue = false;
};

struct OptionArg {
    std::string name;
    std::string shortName;
    OptionType optionType;
    std::string defaultValue;
    bool required = false;
};

using ArgumentVariant = std::variant<PositionalArg, FlagArg, OptionArg, SubcommandArg>;

struct Argument 
{
    const ArgumentType type;
    ArgumentVariant data;
    
    template<typename T>
    const T& get() const {
        return std::get<T>(data);
    }

};

class Parser 
{
public:
    Parser(const std::string& programName) : m_programName(programName)
    {

    }

    bool addPositional(const std::string& name, size_t position)
    {
        return true;
    }

    bool parse(const int argc, const char** argv) const
    {
        return true;
    }

private:

    size_t parseArgument(const std::string& arg, std::optional<std::string> next) const
    {
        return 1;
    }

    size_t parseOption(const std::string& arg, std::optional<std::string> next) const
    {
        return 1;
    }


private:
    std::string m_programName;
    std::unordered_map<std::string, Argument> m_args;
};


} // namespace ArgvNaut
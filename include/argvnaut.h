#pragma once

#include <algorithm>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
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
    size_t position = 0; // 1-indexed
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
    OptionType optionType = OptionType::STRING;
    std::string defaultValue;
    bool required = false;
};

using ArgumentVariant = std::variant<PositionalArg, FlagArg, OptionArg, SubcommandArg>;

struct Argument 
{
    ArgumentType type;
    ArgumentVariant data;
    
    template<typename T>
    const T& get() const {
        return std::get<T>(data);
    }
};

using ParsedValue = std::variant<std::string, bool, int, float>;

class Parser 
{
public:
    Parser(const std::string& programName) : m_programName(programName) {}

    bool addPositional(const std::string& name, size_t position)
    {
        if (m_args.contains(name)) return false;
        if (m_positionalsByPosition.contains(position)) return false;

        PositionalArg pos;
        pos.name = name;
        pos.position = position;

        m_args.emplace(name, Argument{ArgumentType::POSITIONAL, pos});
        m_positionalsByPosition[position] = name;
        return true;
    }

    bool addFlag(const std::string& name, const std::string& shortName = "")
    {
        if (m_args.contains(name)) return false;

        FlagArg flag;
        flag.name = name;
        flag.shortName = shortName;

        m_args.emplace(name, Argument{ArgumentType::FLAG, flag});

        m_longNameLookup["--" + name] = name;
        if (!shortName.empty()) {
            m_shortNameLookup["-" + shortName] = name;
        }
        return true;
    }

    bool addOption(const std::string& name, const std::string& shortName = "",
                   OptionType optionType = OptionType::STRING,
                   const std::string& defaultValue = "", bool required = false)
    {
        if (m_args.contains(name)) return false;

        OptionArg opt;
        opt.name = name;
        opt.shortName = shortName;
        opt.optionType = optionType;
        opt.defaultValue = defaultValue;
        opt.required = required;

        m_args.emplace(name, Argument{ArgumentType::OPTION, opt});

        m_longNameLookup["--" + name] = name;
        if (!shortName.empty()) {
            m_shortNameLookup["-" + shortName] = name;
        }
        return true;
    }

    bool parse(int argc, const char** argv)
    {
        m_results.clear();

        // Set defaults
        for (const auto& [name, arg] : m_args) {
            if (arg.type == ArgumentType::FLAG) {
                m_results[name] = false;
            } else if (arg.type == ArgumentType::OPTION) {
                const auto& opt = arg.get<OptionArg>();
                if (!opt.defaultValue.empty()) {
                    m_results[name] = opt.defaultValue;
                }
            }
        }

        size_t positionalIndex = 1;

        for (int i = 1; i < argc; ++i) {
            std::string token(argv[i]);
            std::optional<std::string> next;
            if (i + 1 < argc) {
                next = std::string(argv[i + 1]);
            }

            if (token.starts_with("--") || token.starts_with("-")) {
                size_t consumed = parseNamedArg(token, next);
                if (consumed == 0) return false;
                i += static_cast<int>(consumed - 1);
            } else {
                auto it = m_positionalsByPosition.find(positionalIndex);
                if (it == m_positionalsByPosition.end()) {
                    return false; // unexpected positional
                }
                m_results[it->second] = token;
                ++positionalIndex;
            }
        }

        // Validate required arguments
        for (const auto& [name, arg] : m_args) {
            if (arg.type == ArgumentType::POSITIONAL) {
                const auto& pos = arg.get<PositionalArg>();
                if (pos.required && !m_results.contains(name)) {
                    return false;
                }
            } else if (arg.type == ArgumentType::OPTION) {
                const auto& opt = arg.get<OptionArg>();
                if (opt.required && !m_results.contains(name)) {
                    return false;
                }
            }
        }

        return true;
    }

    std::optional<std::string> getString(const std::string& name) const
    {
        auto it = m_results.find(name);
        if (it == m_results.end()) return std::nullopt;
        if (auto* s = std::get_if<std::string>(&it->second)) {
            return *s;
        }
        return std::nullopt;
    }

    bool getFlag(const std::string& name) const
    {
        auto it = m_results.find(name);
        if (it == m_results.end()) return false;
        if (auto* b = std::get_if<bool>(&it->second)) {
            return *b;
        }
        return false;
    }

    std::optional<int> getInt(const std::string& name) const
    {
        auto it = m_results.find(name);
        if (it == m_results.end()) return std::nullopt;
        if (auto* v = std::get_if<int>(&it->second)) {
            return *v;
        }
        if (auto* s = std::get_if<std::string>(&it->second)) {
            try { return std::stoi(*s); } catch (...) { return std::nullopt; }
        }
        return std::nullopt;
    }

    std::optional<float> getFloat(const std::string& name) const
    {
        auto it = m_results.find(name);
        if (it == m_results.end()) return std::nullopt;
        if (auto* v = std::get_if<float>(&it->second)) {
            return *v;
        }
        if (auto* s = std::get_if<std::string>(&it->second)) {
            try { return std::stof(*s); } catch (...) { return std::nullopt; }
        }
        return std::nullopt;
    }

    bool has(const std::string& name) const
    {
        return m_results.contains(name);
    }

private:
    // Returns number of tokens consumed (1 for flag, 2 for option with value), 0 on error
    size_t parseNamedArg(const std::string& token, std::optional<std::string> next)
    {
        std::string lookupToken = token;
        std::optional<std::string> inlineValue;

        // Handle --name=value syntax
        if (token.starts_with("--")) {
            auto eqPos = token.find('=');
            if (eqPos != std::string::npos) {
                lookupToken = token.substr(0, eqPos);
                inlineValue = token.substr(eqPos + 1);
            }
        }

        // Resolve argument name from long or short form
        std::string argName;
        auto longIt = m_longNameLookup.find(lookupToken);
        if (longIt != m_longNameLookup.end()) {
            argName = longIt->second;
        } else {
            auto shortIt = m_shortNameLookup.find(lookupToken);
            if (shortIt != m_shortNameLookup.end()) {
                argName = shortIt->second;
            } else {
                return 0; // unknown argument
            }
        }

        auto argIt = m_args.find(argName);
        if (argIt == m_args.end()) return 0;

        const auto& arg = argIt->second;

        if (arg.type == ArgumentType::FLAG) {
            m_results[argName] = true;
            return 1;
        } else if (arg.type == ArgumentType::OPTION) {
            std::string value;
            size_t consumed = 1;

            if (inlineValue) {
                value = *inlineValue;
            } else if (next) {
                value = *next;
                consumed = 2;
            } else {
                return 0; // option requires a value
            }

            m_results[argName] = value;
            return consumed;
        }

        return 0;
    }

private:
    std::string m_programName;
    std::unordered_map<std::string, Argument> m_args;
    std::unordered_map<size_t, std::string> m_positionalsByPosition;
    std::unordered_map<std::string, std::string> m_longNameLookup;  // "--name" -> "name"
    std::unordered_map<std::string, std::string> m_shortNameLookup; // "-n" -> "name"
    std::unordered_map<std::string, ParsedValue> m_results;
};

} // namespace ArgvNaut
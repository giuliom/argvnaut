#pragma once

#include <charconv>
#include <cstdlib>
#include <optional>
#include <sstream>
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
    std::string description;
};

struct SubcommandArg {
    std::string name;
    std::vector<std::string> aliases;
};

struct FlagArg {
    std::string name;
    std::string shortName;
    bool defaultValue = false;
    std::string description;
};

struct OptionArg {
    std::string name;
    std::string shortName;
    OptionType optionType = OptionType::STRING;
    std::string defaultValue;
    bool required = false;
    std::string description;
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

namespace detail {

inline std::optional<int> toInt(const std::string& s)
{
    int value = 0;
    const char* last = s.data() + s.size();
    auto [ptr, ec] = std::from_chars(s.data(), last, value);
    if (ec != std::errc{} || ptr != last) return std::nullopt;
    return value;
}

inline std::optional<float> toFloat(const std::string& s)
{
    if (s.empty()) return std::nullopt;
    char* end = nullptr;
    errno = 0;
    const float value = std::strtof(s.c_str(), &end);
    if (end != s.c_str() + s.size() || errno == ERANGE) return std::nullopt;
    return value;
}

inline std::optional<bool> toBool(const std::string& s)
{
    if (s == "true" || s == "1") return true;
    if (s == "false" || s == "0") return false;
    return std::nullopt;
}

// True for tokens like "-5" or "-3.14" that should be treated as values, not named arguments
inline bool looksNumeric(const std::string& s)
{
    return toFloat(s).has_value();
}

} // namespace detail

class Parser 
{
public:
    Parser(const std::string& programName) : m_programName(programName) {}

    bool addPositional(const std::string& name, size_t position, bool required = true,
                       const std::string& description = "")
    {
        if (position == 0) return false;
        if (m_args.contains(name)) return false;
        if (m_positionalsByPosition.contains(position)) return false;

        PositionalArg pos;
        pos.name = name;
        pos.position = position;
        pos.required = required;
        pos.description = description;

        m_args.emplace(name, Argument{ArgumentType::POSITIONAL, pos});
        m_positionalsByPosition[position] = name;
        return true;
    }

    bool addFlag(const std::string& name, const std::string& shortName = "",
                 const std::string& description = "")
    {
        if (m_args.contains(name)) return false;

        FlagArg flag;
        flag.name = name;
        flag.shortName = shortName;
        flag.description = description;

        m_args.emplace(name, Argument{ArgumentType::FLAG, flag});
        m_namedOrder.push_back(name);

        m_longNameLookup["--" + name] = name;
        if (!shortName.empty()) {
            m_shortNameLookup["-" + shortName] = name;
        }
        return true;
    }

    bool addOption(const std::string& name, const std::string& shortName = "",
                   OptionType optionType = OptionType::STRING,
                   const std::string& defaultValue = "", bool required = false,
                   const std::string& description = "")
    {
        if (m_args.contains(name)) return false;

        OptionArg opt;
        opt.name = name;
        opt.shortName = shortName;
        opt.optionType = optionType;
        opt.defaultValue = defaultValue;
        opt.required = required;
        opt.description = description;

        m_args.emplace(name, Argument{ArgumentType::OPTION, opt});
        m_namedOrder.push_back(name);

        m_longNameLookup["--" + name] = name;
        if (!shortName.empty()) {
            m_shortNameLookup["-" + shortName] = name;
        }
        return true;
    }

    bool parse(int argc, char** argv)
    {
        return parse(argc, const_cast<const char**>(argv));
    }

    bool parse(int argc, const char** argv)
    {
        m_results.clear();
        m_error.clear();

        // Positional positions must be contiguous starting at 1
        for (size_t p = 1; p <= m_positionalsByPosition.size(); ++p) {
            if (!m_positionalsByPosition.contains(p)) {
                return fail("positional argument positions must be contiguous starting at 1");
            }
        }

        // Set defaults, validated against the declared type
        for (const auto& [name, arg] : m_args) {
            if (arg.type == ArgumentType::FLAG) {
                m_results[name] = false;
            } else if (arg.type == ArgumentType::OPTION) {
                const auto& opt = arg.get<OptionArg>();
                if (!opt.defaultValue.empty()) {
                    auto value = convert(opt.defaultValue, opt.optionType);
                    if (!value) {
                        return fail("invalid default value '" + opt.defaultValue +
                                    "' for option '--" + name + "'");
                    }
                    m_results[name] = *value;
                }
            }
        }

        size_t positionalIndex = 1;
        bool positionalsOnly = false;

        for (int i = 1; i < argc; ++i) {
            std::string token(argv[i]);
            std::optional<std::string> next;
            if (i + 1 < argc) {
                next = std::string(argv[i + 1]);
            }

            if (!positionalsOnly && token == "--") {
                positionalsOnly = true; // end-of-options separator
                continue;
            }

            const bool named = !positionalsOnly && token.size() > 1 && token[0] == '-' &&
                               !detail::looksNumeric(token);

            if (named) {
                size_t consumed = parseNamedArg(token, next);
                if (consumed == 0) return false;
                i += static_cast<int>(consumed - 1);
            } else {
                auto it = m_positionalsByPosition.find(positionalIndex);
                if (it == m_positionalsByPosition.end()) {
                    return fail("unexpected positional argument '" + token + "'");
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
                    return fail("missing required positional argument '" + name + "'");
                }
            } else if (arg.type == ArgumentType::OPTION) {
                const auto& opt = arg.get<OptionArg>();
                if (opt.required && !m_results.contains(name)) {
                    return fail("missing required option '--" + name + "'");
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
            return detail::toInt(*s);
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
        if (auto* v = std::get_if<int>(&it->second)) {
            return static_cast<float>(*v);
        }
        if (auto* s = std::get_if<std::string>(&it->second)) {
            return detail::toFloat(*s);
        }
        return std::nullopt;
    }

    bool has(const std::string& name) const
    {
        return m_results.contains(name);
    }

    // Description of the last parse() failure, empty if parse() succeeded
    const std::string& error() const
    {
        return m_error;
    }

    std::string usage() const
    {
        std::string out = "Usage: " + m_programName;
        if (!m_namedOrder.empty()) {
            out += " [options]";
        }
        for (size_t p = 1; p <= m_positionalsByPosition.size(); ++p) {
            auto it = m_positionalsByPosition.find(p);
            if (it == m_positionalsByPosition.end()) break;
            const auto& pos = m_args.at(it->second).get<PositionalArg>();
            out += pos.required ? " <" + pos.name + ">" : " [" + pos.name + "]";
        }
        return out;
    }

    std::string help() const
    {
        constexpr size_t columnWidth = 26;
        std::ostringstream out;
        out << usage() << "\n";

        if (!m_positionalsByPosition.empty()) {
            out << "\nPositional arguments:\n";
            for (size_t p = 1; p <= m_positionalsByPosition.size(); ++p) {
                auto it = m_positionalsByPosition.find(p);
                if (it == m_positionalsByPosition.end()) break;
                const auto& pos = m_args.at(it->second).get<PositionalArg>();
                std::string left = "  " + pos.name;
                out << left << std::string(left.size() < columnWidth ? columnWidth - left.size() : 1, ' ')
                    << pos.description << "\n";
            }
        }

        if (!m_namedOrder.empty()) {
            out << "\nOptions:\n";
            for (const auto& name : m_namedOrder) {
                const auto& arg = m_args.at(name);
                std::string shortName;
                std::string description;
                std::string valueSuffix;
                std::string defaultSuffix;
                if (arg.type == ArgumentType::FLAG) {
                    const auto& flag = arg.get<FlagArg>();
                    shortName = flag.shortName;
                    description = flag.description;
                } else {
                    const auto& opt = arg.get<OptionArg>();
                    shortName = opt.shortName;
                    description = opt.description;
                    valueSuffix = " <value>";
                    if (!opt.defaultValue.empty()) {
                        defaultSuffix = " (default: " + opt.defaultValue + ")";
                    }
                }
                std::string left = "  ";
                left += shortName.empty() ? "    " : "-" + shortName + ", ";
                left += "--" + name + valueSuffix;
                out << left << std::string(left.size() < columnWidth ? columnWidth - left.size() : 1, ' ')
                    << description << defaultSuffix << "\n";
            }
        }

        return out.str();
    }

private:
    bool fail(const std::string& message)
    {
        m_error = message;
        return false;
    }

    static std::optional<ParsedValue> convert(const std::string& value, OptionType type)
    {
        switch (type) {
            case OptionType::STRING:
                return ParsedValue{value};
            case OptionType::INTEGER:
                if (auto v = detail::toInt(value)) return ParsedValue{*v};
                return std::nullopt;
            case OptionType::FLOAT:
                if (auto v = detail::toFloat(value)) return ParsedValue{*v};
                return std::nullopt;
            case OptionType::BOOLEAN:
                if (auto v = detail::toBool(value)) return ParsedValue{*v};
                return std::nullopt;
        }
        return std::nullopt;
    }

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
                fail("unknown argument '" + token + "'");
                return 0;
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
                fail("option '" + token + "' requires a value");
                return 0;
            }

            const auto& opt = arg.get<OptionArg>();
            auto converted = convert(value, opt.optionType);
            if (!converted) {
                fail("invalid value '" + value + "' for option '--" + argName + "'");
                return 0;
            }

            m_results[argName] = *converted;
            return consumed;
        }

        fail("argument '" + token + "' cannot be used as a named argument");
        return 0;
    }

private:
    std::string m_programName;
    std::unordered_map<std::string, Argument> m_args;
    std::vector<std::string> m_namedOrder; // flags/options in declaration order, for help()
    std::unordered_map<size_t, std::string> m_positionalsByPosition;
    std::unordered_map<std::string, std::string> m_longNameLookup;  // "--name" -> "name"
    std::unordered_map<std::string, std::string> m_shortNameLookup; // "-n" -> "name"
    std::unordered_map<std::string, ParsedValue> m_results;
    std::string m_error;
};

} // namespace ArgvNaut
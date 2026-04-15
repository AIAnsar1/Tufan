#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <format>
#include <stdexcept>

#include "EnvParser.hh"

namespace Tufan::Configuration
{
    EnvParser::EnvMap EnvParser::parse(const std::filesystem::path &path)
    {
        if (!std::filesystem::exists(path))
        {
            throw Tufan::Exceptions::ConfigException(std::format("Env file not found: {}", path.string()));
        }
        std::ifstream file(path);

        if (!file.is_open())
            {
            throw Tufan::Exceptions::ConfigException(std::format("Cannot open env file: {}", path.string()));
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        return parseString(ss.str());
    }

    EnvParser::EnvMap EnvParser::parseString(std::string_view content)
    {
        EnvMap result;
        std::istringstream stream{std::string(content)};
        std::string line;

        while (std::getline(stream, line))
        {
            auto parsed = parseLine(line, result);

            if (parsed)
            {
                result.emplace(std::move(parsed->first), std::move(parsed->second));
            }
        }
        return result;
    }


std::optional<std::pair<std::string, std::string>>
EnvParser::parseLine(std::string_view line, const EnvMap& existing) {
    // Trim leading whitespace
    auto start = line.find_first_not_of(" \t\r\n");
    if (start == std::string_view::npos) return std::nullopt;
    line = line.substr(start);

    // Skip comments
    if (line.starts_with('#')) return std::nullopt;
    if (line.starts_with("export ")) line = line.substr(7);

    // Find '='
    auto eq = line.find('=');
    if (eq == std::string_view::npos) return std::nullopt;

    std::string key   = std::string(line.substr(0, eq));
    std::string value = std::string(line.substr(eq + 1));

    // Trim key
    while (!key.empty() && std::isspace(static_cast<unsigned char>(key.back()))) {
        key.pop_back();
    }

    // Trim value trailing comment
    value = stripQuotes(value);
    value = interpolate(value, existing);

    if (key.empty()) return std::nullopt;
    return std::make_pair(std::move(key), std::move(value));
}

std::string EnvParser::stripQuotes(std::string_view value) {
    // Trim leading/trailing spaces
    auto start = value.find_first_not_of(' ');
    auto end   = value.find_last_not_of(' ');
    if (start == std::string_view::npos) return "";
    value = value.substr(start, end - start + 1);

    // Double quotes — сохраняем пробелы, убираем inline-комментарии
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        return std::string(value.substr(1, value.size() - 2));
    }

    // Single quotes — буквально
    if (value.size() >= 2 && value.front() == '\'' && value.back() == '\'') {
        return std::string(value.substr(1, value.size() - 2));
    }

    // Без кавычек — обрезаем inline-комментарий
    auto commentPos = value.find(" #");
    if (commentPos != std::string_view::npos) {
        value = value.substr(0, commentPos);
    }

    return std::string(value);
}

    std::string EnvParser::interpolate(std::string_view value, const EnvMap& vars)
    {
        std::string result(value);
        static const std::regex varPattern(R"(\$\{([A-Za-z_][A-Za-z0-9_]*)\}|\$([A-Za-z_][A-Za-z0-9_]*))");
        std::string output;
        auto it  = std::sregex_iterator(result.begin(), result.end(), varPattern);
        auto end = std::sregex_iterator();
        std::size_t lastPos = 0;

        for (; it != end; ++it)
        {
            const auto& match = *it;
            output += result.substr(lastPos, match.position() - lastPos);
            std::string varName = match[1].matched ? match[1].str() : match[2].str();

            if (auto found = vars.find(varName); found != vars.end())
            {
                output += found->second;
            }
            else if (const char* env = std::getenv(varName.c_str()))
            {
                output += env;
            }
            lastPos = match.position() + match.length();
        }
        output += result.substr(lastPos);
        return output;
    }
}

































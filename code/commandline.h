#pragma once
#include <string>
#include <map>
#include <set>
#include <filesystem>

class CommandLine {
public:
    std::set<std::wstring> mOptions;
    std::map<std::wstring, std::wstring> mArguments;

    CommandLine(int argc, wchar_t *argv[], std::set<std::wstring> const &arguments, std::set<std::wstring> const &options);
    bool HasOption(std::wstring const &option);
    bool HasArgument(std::wstring const &argument);
    std::wstring GetArgumentString(std::wstring const &argument, std::wstring const &defaultValue = std::wstring());
    std::filesystem::path GetArgumentPath(std::wstring const &argument, std::filesystem::path const &defaultValue = std::filesystem::path());
    int GetArgumentInt(std::wstring const &argument, int defaultValue = -1);
    float GetArgumentFloat(std::wstring const &argument, float defaultValue = 0.0f);
};

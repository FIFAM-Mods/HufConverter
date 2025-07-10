#include "commandline.h"
#include "utils.h"

CommandLine::CommandLine(int argc, wchar_t *argv[], std::set<std::wstring> const &arguments, std::set<std::wstring> const &options) {
    std::set<std::wstring> _arguments;
    std::set<std::wstring> _options;
    for (auto const &s : arguments)
        _arguments.insert(ToLower(s));
    for (auto const &s : options)
        _options.insert(ToLower(s));
    for (int i = 1; i < argc; i++) {
        std::wstring arg = argv[i];
        if (arg.starts_with('-')) {
            arg = ToLower(arg.substr(1));
            if (_arguments.contains(arg)) {
                if ((i + 1) < argc)
                    mArguments[arg] = argv[i + 1];
                else
                    break;
            }
            else if (_options.contains(arg))
                mOptions.insert(arg);
        }
    }
}

bool CommandLine::HasOption(std::wstring const &option) {
    return mOptions.contains(ToLower(option));
}

bool CommandLine::HasArgument(std::wstring const &argument) {
    return mArguments.contains(ToLower(argument));
}

std::wstring CommandLine::GetArgumentString(std::wstring const &argument, std::wstring const &defaultValue) {
    auto l = ToLower(argument);
    if (HasArgument(l))
        return mArguments[l];
    return defaultValue;
}

std::filesystem::path CommandLine::GetArgumentPath(std::wstring const &argument, std::filesystem::path const &defaultValue) {
    auto l = ToLower(argument);
    if (HasArgument(l))
        return mArguments[l];
    return defaultValue;
}

int CommandLine::GetArgumentInt(std::wstring const &argument, int defaultValue) {
    auto l = ToLower(argument);
    if (HasArgument(l)) {
        try {
            return std::stoi(mArguments[l]);
        }
        catch (...) {}
    }
    return defaultValue;
}

float CommandLine::GetArgumentFloat(std::wstring const &argument, float defaultValue) {
    auto l = ToLower(argument);
    if (HasArgument(l)) {
        try {
            return std::stof(mArguments[l]);
        }
        catch (...) {}
    }
    return defaultValue;
}

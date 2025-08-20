#pragma once
#include <cstdio>
#include <string>
#include <vector>

std::wstring AtoW(std::string const &str);
std::string WtoA(std::wstring const &str);
std::wstring ToUpper(std::wstring const &str);
std::wstring ToLower(std::wstring const &str);
void Trim(std::wstring &str);
unsigned int Hash(std::string const &str);
std::string ToUTF8(std::wstring const &wstr);
std::wstring ToUTF16(std::string const &str);
void ConvertUTF16ToWindows1251(std::wstring &str);
void ConvertWindows1251ToUTF16(std::wstring &str);
bool IsNumber(const std::wstring &str);
std::wstring ReplaceAll(std::wstring const &input, std::vector<std::pair<std::wstring, std::wstring>> const &replacements);

template<typename T>
T SafeConvertInt(std::wstring const &str, bool isHex = false) {
    T result = 0;
    try {
        result = static_cast<T>(std::stoull(str, 0, isHex ? 16 : 10));
    }
    catch (...) {}
    return result;
}

template<typename T>
T SafeConvertInt(std::string const &str, bool isHex = false) {
    T result = 0;
    try {
        result = static_cast<T>(std::stoull(str, 0, isHex ? 16 : 10));
    }
    catch (...) {}
    return result;
}

class FormattingUtils {
    static const unsigned int BUF_SIZE = 10;
    static unsigned int currentBuf;
    static char buf[BUF_SIZE][4096];
    static unsigned int currentBufW;
    static wchar_t bufW[BUF_SIZE][4096];
public:
    template<typename T> static T const &Arg(T const &arg) { return arg; }
    static char const *Arg(std::string const &arg) { return arg.c_str(); }
    static wchar_t const *Arg(std::wstring const &arg) { return arg.c_str(); }
    static char *GetBuf();
    static wchar_t *GetBufW();
    static void WindowsMessageBoxA(char const *msg, char const *title, unsigned int icon);
    static void WindowsMessageBoxW(wchar_t const *msg, wchar_t const *title, unsigned int icon);
};

template<typename ...ArgTypes>
char *FormatStatic(const std::string &format, ArgTypes... args) {
    char *buf = FormattingUtils::GetBuf();
    snprintf(buf, 4096, format.c_str(), FormattingUtils::Arg(args)...);
    return buf;
}

template<typename ...ArgTypes>
std::string Format(const std::string &format, ArgTypes... args) {
    return FormatStatic(format, FormattingUtils::Arg(args)...);
}

template<typename ...ArgTypes>
wchar_t *FormatStatic(const std::wstring &format, ArgTypes... args) {
    wchar_t *buf = FormattingUtils::GetBufW();
    _snwprintf(buf, 4096, format.c_str(), FormattingUtils::Arg(args)...);
    return buf;
}

template<typename ...ArgTypes>
std::wstring Format(const std::wstring &format, ArgTypes... args) {
    return FormatStatic(format, FormattingUtils::Arg(args)...);
}

template <typename... ArgTypes>
bool Message(std::string const &format, ArgTypes... args) {
    FormattingUtils::WindowsMessageBoxA(FormatStatic(format, args...), "Message", 0);
    return false;
}

template <typename... ArgTypes>
bool Error(std::string const &format, ArgTypes... args) {
    FormattingUtils::WindowsMessageBoxA(FormatStatic(format, args...), "Error", 2);
    return false;
}

template <typename... ArgTypes>
bool Warning(std::string const &format, ArgTypes... args) {
    FormattingUtils::WindowsMessageBoxA(FormatStatic(format, args...), "Warning", 1);
    return false;
}

template <typename... ArgTypes>
bool Message(std::wstring const &format, ArgTypes... args) {
    FormattingUtils::WindowsMessageBoxW(FormatStatic(format, args...), L"Message", 0);
    return false;
}

template <typename... ArgTypes>
bool Error(std::wstring const &format, ArgTypes... args) {
    FormattingUtils::WindowsMessageBoxW(FormatStatic(format, args...), L"Error", 2);
    return false;
}

template <typename... ArgTypes>
bool Warning(std::wstring const &format, ArgTypes... args) {
    FormattingUtils::WindowsMessageBoxW(FormatStatic(format, args...), L"Warning", 1);
    return false;
}

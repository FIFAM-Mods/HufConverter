#include "utils.h"
#include <Windows.h>
#include "utils.h"

unsigned int FormattingUtils::currentBuf = 0;
char FormattingUtils::buf[FormattingUtils::BUF_SIZE][4096];
unsigned int FormattingUtils::currentBufW = 0;
wchar_t FormattingUtils::bufW[FormattingUtils::BUF_SIZE][4096];

std::wstring AtoW(std::string const &str) {
    std::wstring result;
    result.resize(str.size());
    for (unsigned int i = 0; i < str.size(); i++)
        result[i] = static_cast<wchar_t>(static_cast<unsigned char>(str[i]));
    return result;
}

std::string WtoA(std::wstring const &str) {
    std::string result;
    result.resize(str.size());
    for (unsigned int i = 0; i < str.size(); i++)
        result[i] = static_cast<char>(static_cast<unsigned char>(str[i]));
    return result;
}

std::wstring ToUpper(std::wstring const &str) {
    std::wstring result;
    for (size_t i = 0; i < str.length(); i++)
        result += toupper(static_cast<unsigned short>(str[i]));
    return result;
}

std::wstring ToLower(std::wstring const &str) {
    std::wstring result;
    for (size_t i = 0; i < str.length(); i++)
        result += tolower(static_cast<unsigned short>(str[i]));
    return result;
}

void Trim(std::wstring &str) {
    size_t start = str.find_first_not_of(L" \t\r\n");
    if (start != std::wstring::npos)
        str = str.substr(start);
    size_t end = str.find_last_not_of(L" \t\r\n");
    if (end != std::wstring::npos)
        str = str.substr(0, end + 1);
}

unsigned int Hash(std::string const &str) {
    unsigned int hash = 0;
    for (auto const &c : str) {
        hash += c;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

std::string ToUTF8(std::wstring const &wstr) {
    if (wstr.empty())
        return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::wstring ToUTF16(std::string const &str) {
    if (str.empty())
        return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring strTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &strTo[0], size_needed);
    return strTo;
}

bool IsNumber(const std::wstring &str) {
    if (str.empty())
        return false;
    for (wchar_t c : str) {
        if (!iswdigit(c))
            return false;
    }
    return true;
}

std::wstring ReplaceAll(std::wstring const &input, std::vector<std::pair<std::wstring, std::wstring>> const &replacements) {
    if (replacements.empty())
        return input;
    std::wstring output;
    output.reserve(input.size());
    size_t pos = 0;
    while (pos < input.size()) {
        bool matched = false;
        for (auto const &[key, value] : replacements) {
            if (!key.empty() &&
                pos + key.size() <= input.size() &&
                input.compare(pos, key.size(), key) == 0) {
                output.append(value);
                pos += key.size();
                matched = true;
                break;
            }
        }
        if (!matched) {
            output.push_back(input[pos]);
            ++pos;
        }
    }
    return output;
}

UINT MessageIcon(unsigned int iconType) {
    if (iconType == 1)
        return MB_ICONWARNING;
    else if (iconType == 2)
        return MB_ICONERROR;
    return MB_ICONINFORMATION;
}

void FormattingUtils::WindowsMessageBoxA(char const *msg, char const *title, unsigned int icon) {
    MessageBoxA(GetActiveWindow(), msg, title, MessageIcon(icon));
}

void FormattingUtils::WindowsMessageBoxW(wchar_t const *msg, wchar_t const *title, unsigned int icon) {
    MessageBoxW(GetActiveWindow(), msg, title, MessageIcon(icon));
}

char *FormattingUtils::GetBuf() {
    char *result = buf[currentBuf];
    currentBuf += 1;
    if (currentBuf >= BUF_SIZE)
        currentBuf = 0;
    return result;
}

wchar_t *FormattingUtils::GetBufW() {
    wchar_t *result = bufW[currentBufW];
    currentBufW += 1;
    if (currentBufW >= BUF_SIZE)
        currentBufW = 0;
    return result;
}

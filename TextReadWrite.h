#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <fcntl.h>
#include <io.h>
#include <Windows.h>

enum eEncoding {
    ENCODING_ASCII,
    ENCODING_UTF8_BOM,
    ENCODING_UTF16LE_BOM
};

class TextFileWriter {
    FILE *mFile = nullptr;
public:
    TextFileWriter(std::filesystem::path const &filename, eEncoding encoding = ENCODING_UTF8_BOM);
    ~TextFileWriter();
    void Close();
    bool Available();
    void Write(std::wstring const &str);
    void WriteLine(std::wstring const &line);
    void Write(wchar_t value);
    void WriteNewLine();
};

class TextFileReader {
    std::vector<std::wstring> mLines;
    bool mAvailable = false;
public:
    std::vector<std::wstring> &Lines();
    void Close();
    bool Available();
    TextFileReader(std::filesystem::path const &filename);
    ~TextFileReader();
};

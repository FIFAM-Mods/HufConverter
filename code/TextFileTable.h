#pragma once
#include <string>
#include <vector>
#include <filesystem>

enum eEncoding {
    ENCODING_ANSI,
    ENCODING_UTF8,
    ENCODING_UTF8_BOM,
    ENCODING_UTF16LE_BOM,
    ENCODING_UTF16BE_BOM
};

class TextFileTable {
    std::vector<std::vector<std::wstring>> mCells;

    static std::wstring Unquoted(std::wstring const &str);
    static std::wstring Quoted(std::wstring const &str, wchar_t separator);
    size_t NumRowsToWrite() const;
public:
    size_t NumRows() const;
    size_t NumColumns(size_t row) const;
    bool IsConsistent() const;
    size_t MaxColumns() const;
    std::vector<std::vector<std::wstring>> const &Rows() const;
    std::vector<std::wstring> const &Row(size_t row) const;
    std::wstring const &Cell(size_t column, size_t row) const;
    void AddRow(std::vector<std::wstring> const &row);
    void Clear();
    bool Read(std::filesystem::path const &filename, wchar_t separator = L',');
    bool Write(std::filesystem::path const &filename, wchar_t separator = L',', eEncoding encoding = ENCODING_UTF8_BOM);
};

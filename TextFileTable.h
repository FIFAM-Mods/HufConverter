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

class TextFileTable {
    std::vector<std::vector<std::wstring>> mCells;

    static std::wstring Unquoted(std::wstring const &str) {
        if (str.size() > 1 && str[0] == L'"' && str[str.size() - 1] == L'"') {
            std::wstring unquoted = str.substr(1, str.size() - 2);
            std::wstring result;
            for (size_t i = 0; i < unquoted.size(); i++) {
                if (unquoted[i] == '"' && (i + 1) < unquoted.size() && unquoted[i + 1] == '"')
                    i++;
                result += unquoted[i];
            }
            return result;
        }
        return str;
    }

    static std::wstring Quoted(std::wstring const &str, wchar_t separator) {
        bool addQuotes = false;
        for (wchar_t c : str) {
            if (c == L'\r' || c == L'\n' || c == L'"' || c == separator) {
                addQuotes = true;
                break;
            }
        }
        if (!addQuotes)
            return str;
        std::wstring result;
        result += L'"';
        for(wchar_t c : str) {
            result += c;
            if (c == L'"')
                result += c;
        }
        result += L'"';
        return result;
    }

    size_t NumRowsToWrite() const {
        size_t result = 0;
        for (size_t r = 0; r < mCells.size(); r++) {
            bool validRow = false;
            if (!mCells[r].empty()) {
                for (size_t c = 0; c < mCells[r].size(); c++) {
                    if (!mCells[r][c].empty()) {
                        validRow = true;
                        break;
                    }
                }
            }
            if (validRow)
                result = r + 1;
        }
        return result;
    }
public:
    size_t NumRows() const {
        return mCells.size();
    }

    size_t NumColumns(size_t row) const {
        if (row < mCells.size())
            return mCells[row].size();
        return 0;
    }

    bool IsConsistent() const {
        if (mCells.size() > 1) {
            for (size_t i = 1; i < mCells.size(); i++) {
                if (mCells[i - 1].size() != mCells[i].size())
                    return false;
            }
        }
        return true;
    }

    size_t MaxColumns() const {
        size_t maxColumns = 0;
        for (auto const &row : mCells) {
            if (row.size() > maxColumns)
                maxColumns = row.size();
        }
        return maxColumns;
    }

    std::vector<std::vector<std::wstring>> const &Rows() const {
        return mCells;
    }

    std::vector<std::wstring> const &Row(size_t row) const {
        static std::vector<std::wstring> emptyRow;
        if (row < mCells.size())
            return mCells[row];
        return emptyRow;
    }

    std::wstring const &Cell(size_t column, size_t row) const {
        static std::wstring emptyCell;
        if (row < mCells.size() && column < mCells[row].size())
            return mCells[row][column];
        return emptyCell;
    }

    void AddRow(std::vector<std::wstring> const &row) {
        mCells.push_back(row);
    }

    void Clear() {
        mCells.clear();
    }

    bool Read(std::filesystem::path const &filename, wchar_t separator = L',') {
        Clear();
        FILE *file = _wfopen(filename.c_str(), L"rb");
        if (file) {
            fseek(file, 0, SEEK_END);
            auto fileSizeWithBom = ftell(file);
            if (fileSizeWithBom == 0) {
                fclose(file);
                return false;
            }
            fseek(file, 0, SEEK_SET);
            enum class encoding { ascii, utf8, utf16le, utf16be } enc = encoding::ascii;
            long numBytesToSkip = 0;
            if (fileSizeWithBom >= 2) {
                unsigned char bom[3];
                bom[0] = 0;
                fread(&bom, 1, 2, file);
                fseek(file, 0, SEEK_SET);
                if (bom[0] == 0xFE && bom[1] == 0xFF) {
                    enc = encoding::utf16be;
                    numBytesToSkip = 2;
                }
                else if (bom[0] == 0xFF && bom[1] == 0xFE) {
                    enc = encoding::utf16le;
                    numBytesToSkip = 2;
                }
                else if (fileSizeWithBom >= 3) {
                    bom[0] = 0;
                    fread(&bom, 1, 3, file);
                    fseek(file, 0, SEEK_SET);
                    if (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
                        enc = encoding::utf8;
                        numBytesToSkip = 3;
                    }
                }
            }
            long totalSize = fileSizeWithBom - numBytesToSkip;
            char *fileData = new char[totalSize];
            fseek(file, numBytesToSkip, SEEK_SET);
            auto numRead = fread(fileData, 1, totalSize, file);
            fclose(file);
            if (numRead != totalSize) {
                delete[] fileData;
                return false;
            }
            long numWideChars = 0;
            switch (enc) {
            case encoding::ascii:
                numWideChars = totalSize;
                break;
            case encoding::utf8:
                numWideChars = MultiByteToWideChar(CP_UTF8, 0, fileData, totalSize, 0, 0);
                break;
            case encoding::utf16le:
            case encoding::utf16be:
                numWideChars = totalSize / 2;
                break;
            }
            if (numWideChars == 0) {
                delete[] fileData;
                return true;
            }
            wchar_t *data = new wchar_t[numWideChars];
            memset(data, 0, numWideChars * sizeof(wchar_t));

            switch (enc) {
            case encoding::ascii:
                MultiByteToWideChar(1252, 0, fileData, totalSize, data, numWideChars);
                break;
            case encoding::utf8:
                MultiByteToWideChar(CP_UTF8, 0, fileData, totalSize, data, numWideChars);
                break;
            case encoding::utf16le:
            case encoding::utf16be:
                memcpy(data, fileData, totalSize);
                break;
            }
            delete[] fileData;

            if (enc == encoding::utf16be) {
                for (long i = 0; i < numWideChars; i++)
                    data[i] = (data[i] >> 8) | (data[i] << 8);
            }

            std::vector<std::wstring> mLines;
            std::wstring currentLine;
            unsigned int numQuotes = 0;
            for (long i = 0; i < numWideChars; i++) {
                if (data[i] == L'\n') {
                    if (numQuotes % 2)
                        currentLine += data[i];
                    else {
                        mLines.push_back(currentLine);
                        currentLine.clear();
                    }
                }
                else if (data[i] == L'\r') {
                    if (numQuotes % 2)
                        currentLine += data[i];
                    if ((i + 1) < numWideChars && data[i + 1] == L'\n') {
                        i++;
                        if (numQuotes % 2)
                            currentLine += data[i];
                    }
                    if ((numQuotes % 2) == 0) {
                        mLines.push_back(currentLine);
                        currentLine.clear();
                    }
                }
                else {
                    if (data[i] == L'"')
                        numQuotes++;
                    currentLine += data[i];
                }
            }
            if (!currentLine.empty() || !mLines.empty())
                mLines.push_back(currentLine);
            delete[] data;

            mCells.resize(mLines.size());

            for (size_t l = 0; l < mLines.size(); l++) {
                numQuotes = 0;
                auto const &line = mLines[l];
                std::wstring value;
                for (size_t i = 0; i < line.size(); i++) {
                    if (line[i] == L'"')
                        numQuotes++;
                    else if (line[i] == separator && ((numQuotes % 2) == 0)) {
                        mCells[l].push_back(Unquoted(value));
                        value.clear();
                        continue;
                    }
                    value += line[i];
                }
                mCells[l].push_back(Unquoted(value));
                value.clear();
            }

            mCells.resize(NumRowsToWrite());
        }
        return true;
    }

    bool Write(std::filesystem::path const &filename, wchar_t separator = L',', eEncoding encoding = ENCODING_UTF8_BOM) {
        if (filename.empty())
            return false;
        auto parentPath = filename.parent_path();
        if (!parentPath.empty() && !std::filesystem::exists(parentPath)) {
            std::error_code ec;
            if (!std::filesystem::create_directories(parentPath, ec))
                return false;
        }
        bool unicode = encoding != ENCODING_ASCII;
        FILE *mFile = _wfopen(filename.c_str(), unicode ? L"wb" : L"wt");
        if (!mFile)
            return false;
        if (unicode) {
            if (encoding == ENCODING_UTF16LE_BOM) {
                unsigned char bom[2] = { 0xFF, 0xFE };
                fwrite(bom, 2, 1, mFile);
            }
            else {
                unsigned char bom[3] = { 0xEF, 0xBB, 0xBF };
                fwrite(bom, 3, 1, mFile);
            }
            fclose(mFile);
            mFile = _wfopen(filename.c_str(), L"a");
            if (!mFile)
                return false;
            _setmode(_fileno(mFile), encoding == ENCODING_UTF16LE_BOM ? _O_U16TEXT : _O_U8TEXT);
            size_t numRowsToWrite = NumRowsToWrite();
            size_t numColumnsToWrite = MaxColumns();
            if (numColumnsToWrite > 0) {
                for (size_t r = 0; r < numRowsToWrite; r++) {
                    for (size_t c = 0; c < numColumnsToWrite; c++) {
                        if (c != 0)
                            fputwc(separator, mFile);
                        fputws(Quoted(Cell(c, r), separator).c_str(), mFile);
                    }
                    fputwc(L'\n', mFile);
                }
            }
            else
                fputwc(L'\n', mFile);
        }
        fclose(mFile);
        return true;
    }

    static void Test() {
        TextFileTable table1;
        bool tableRead = table1.Read("Table.csv");
        if (tableRead && table1.IsConsistent() && table1.MaxColumns() == 2) {
            for (auto const &row : table1.Rows())
                std::wcout << row[0] << L" - " << row[1] << std::endl;
        }
        
        TextFileTable table2;
        table2.AddRow({ L"Apples", std::to_wstring(5) });
        table2.AddRow({ L"Bananas", std::to_wstring(3) });
        table2.Write("NewTable.csv");
    }
};

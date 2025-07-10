#include "TextReadWrite.h"

TextFileWriter::TextFileWriter(std::filesystem::path const &filename, eEncoding encoding) {
    auto parentPath = filename.parent_path();
    if (!parentPath.empty() && !exists(parentPath))
        create_directories(parentPath);
    bool unicode = encoding != ENCODING_ASCII;
    mFile = _wfopen(filename.c_str(), unicode ? L"wb" : L"wt");
    if (mFile && unicode) {
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
        if (mFile)
            _setmode(_fileno(mFile), encoding == ENCODING_UTF16LE_BOM ? _O_U16TEXT : _O_U8TEXT);
    }
}

TextFileWriter::~TextFileWriter() {
    Close();
}

void TextFileWriter::Close() {
    if (mFile) {
        fclose(mFile);
        mFile = nullptr;
    }
}

bool TextFileWriter::Available() {
    return mFile != nullptr;
}

void TextFileWriter::Write(std::wstring const &str) {
    fputws(str.c_str(), mFile);
}

void TextFileWriter::WriteLine(std::wstring const &line) {
    Write(line);
    WriteNewLine();
}

void TextFileWriter::Write(wchar_t value) {
    fputwc(value, mFile);
}

void TextFileWriter::WriteNewLine() {
    Write(L'\n');
}

std::vector<std::wstring> &TextFileReader::Lines() {
    return mLines;
}

void TextFileReader::Close() {
    mLines.clear();
    mAvailable = false;
}

bool TextFileReader::Available() {
    return mAvailable;
}

TextFileReader::TextFileReader(std::filesystem::path const &filename) {
    FILE *file = _wfopen(filename.c_str(), L"rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        auto fileSizeWithBom = ftell(file);
        if (fileSizeWithBom == 0) {
            fclose(file);
            return;
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
        if (numRead != totalSize)
            return;
        mAvailable = true;
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
        if (numWideChars == 0)
            return;
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
        mLines.push_back(currentLine);
        delete[] data;
    }
}

TextFileReader::~TextFileReader() {
    Close();
}

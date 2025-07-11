#include "commandline.h"
#include "utils.h"
#include "Text.h"
#include "message.h"
#include "TextFileTable.h"
#include "xlsxwriter.h"
#include "xlnt\xlnt.hpp"

wchar_t const *version = L"1.00";

int wmain(int argc, wchar_t *argv[]) {
    enum ErrorType {
        NONE = 0,
        UNKNOWN_OPERATION_TYPE = 1,
        NO_INPUT_PATH = 2,
        INVALID_INPUT_PATH = 3,
        UNABLE_TO_CREATE_OUTPUT_FOLDER = 4,
        INVALID_KEYS_PATH = 5,
        INVALID_GAME = 6,
        INPUT_FILE_READING_ERROR = 7,
        OUTPUT_FILE_WRITING_ERROR = 7,
        ERROR_OTHER = 8
    };
    enum eFileType { FILETYPE_NOTSET, FILETYPE_HUF, FILETYPE_XLSX, FILETYPE_TXT, FILETYPE_CSV, FILETYPE_TSV, FILETYPE_TR };
    struct FileTypeInfo {
        wchar_t separator;
        wchar_t const *extension;
        eEncoding encoding;
    };
    FileTypeInfo fileType[] = {
        { L'\0', L"", ENCODING_UTF8_BOM },
        { L'\0', L".huf", ENCODING_UTF8_BOM },
        { L'\0', L".xlsx", ENCODING_UTF8_BOM },
        { L'\t', L".txt", ENCODING_UTF16LE_BOM },
        { L',', L".csv", ENCODING_UTF8_BOM },
        { L'\t', L".tsv", ENCODING_UTF8_BOM },
        { L'|', L".tr", ENCODING_UTF8_BOM },
    };
    CommandLine cmd(argc, argv, { L"game", L"g", L"input", L"i", L"output", L"o", L"keys", L"k",
        L"locale", L"language", L"l", L"separator", L"s" },
        { L"silent", L"hashes", L"stats" } );
    SetMessageDisplayType(cmd.HasOption(L"silent") ? MessageDisplayType::MSG_CONSOLE : MessageDisplayType::MSG_MESSAGE_BOX);
    std::pair<eFileType, eFileType> format = { FILETYPE_NOTSET, FILETYPE_NOTSET };
    if (argc >= 2) {
        std::wstring opTypeStr = ToLower(argv[1]);
        if (opTypeStr == L"huf2xlsx" || opTypeStr == L"huf2xls")
            format = { FILETYPE_HUF, FILETYPE_XLSX };
        else if (opTypeStr == L"huf2txt")
            format = { FILETYPE_HUF, FILETYPE_TXT };
        else if (opTypeStr == L"huf2csv")
            format = { FILETYPE_HUF, FILETYPE_CSV };
        else if (opTypeStr == L"huf2tsv")
            format = { FILETYPE_HUF, FILETYPE_TSV };
        else if (opTypeStr == L"huf2tr")
            format = { FILETYPE_HUF, FILETYPE_TR };
        else if (opTypeStr == L"xlsx2huf" || opTypeStr == L"xls2huf")
            format = { FILETYPE_XLSX, FILETYPE_HUF };
        else if (opTypeStr == L"txt2huf")
            format = { FILETYPE_TXT, FILETYPE_HUF };
        else if (opTypeStr == L"csv2huf")
            format = { FILETYPE_CSV, FILETYPE_HUF };
        else if (opTypeStr == L"tsv2huf")
            format = { FILETYPE_TSV, FILETYPE_HUF };
        else if (opTypeStr == L"tr2huf")
            format = { FILETYPE_TR, FILETYPE_HUF };
    }
    if (format.first == FILETYPE_NOTSET || format.second == FILETYPE_NOTSET) {
        ErrorMessage(L"Unknown operation type\nPlease use HufConverterGUI.py if you don't understand how to work with command-line tool");
        return ErrorType::UNKNOWN_OPERATION_TYPE;
    }
    std::filesystem::path in, out, keysPath = L"keys.txt";
    eGame game = GAME_FM09;
    unsigned int localeID = 1;
    wchar_t separator = 0;
    bool hashes = (format.second != FILETYPE_TR) ? cmd.HasOption(L"hashes") : false;
    bool stats = cmd.HasOption(L"stats");
    for (auto const &[arg, value] : cmd.mArguments) {
        if (arg == L"game" || arg == L"g") {
            std::wstring gameStr = ToLower(value);
            std::map<std::wstring, eGame> gameName = {
                { L"tcm2005", GAME_TCM2005 },
                { L"fm06", GAME_FM06 },
                { L"fm07", GAME_FM06 },
                { L"fm08", GAME_FM06 },
                { L"fm09", GAME_FM09 },
                { L"fm10", GAME_FM09 },
                { L"fm11", GAME_FM09 },
                { L"fm12", GAME_FM09 },
                { L"fm13", GAME_FM09 },
                { L"fm14", GAME_FM09 }
            };
            if (!gameName.contains(gameStr)) {
                ErrorMessage(L"Invalid game value");
                return ErrorType::INVALID_GAME;
            }
            game = gameName[gameStr];
        }
        else if (arg == L"input" || arg == L"i") {
            in = value;
            if (!std::filesystem::exists(in)) {
                ErrorMessage(L"Input path does not exist");
                return ErrorType::INVALID_INPUT_PATH;
            }
        }
        else if (arg == L"output" || arg == L"o") {
            out = value;
            if (out.has_parent_path() && !exists(out.parent_path())) {
                std::error_code ec;
                if (!std::filesystem::create_directories(out.parent_path(), ec)) {
                    ErrorMessage(L"Unable to create output folder");
                    return ErrorType::UNABLE_TO_CREATE_OUTPUT_FOLDER;
                }
            }
        }
        else if (arg == L"keys" || arg == L"k") {
            if (value.empty() || ToLower(value) == L"none")
                keysPath.clear();
            else {
                keysPath = value;
                if (!std::filesystem::exists(keysPath)) {
                    ErrorMessage(L"Keys path does not exist");
                    return ErrorType::INVALID_KEYS_PATH;
                }
            }
        }
        else if (arg == L"separator" || arg == L"s") {
            if (!value.empty())
                separator = value[0];
        }
        else if (arg == L"locale" || arg == L"language" || arg == L"l") {
            try {
                localeID = std::stoi(value);
            }
            catch (...) {}
        }
    }
    if (in.empty()) {
        ErrorMessage(L"Input path is not specified");
        return ErrorType::NO_INPUT_PATH;
    }
    if (out.empty()) {
        out = in;
        out.replace_extension(fileType[format.second].extension);
    }

    bool success = false;
    CText text;
    text.m_nLanguageID = localeID;

    if (format.first == FILETYPE_HUF)
        success = text.LoadTranslationsFile(in.c_str(), game);
    else {
        std::map<unsigned int, std::wstring> strings;
        auto AddKeyAndValue = [&strings](std::wstring const &key, std::wstring const &value) {
            unsigned int hash = 0;
            bool hasPrefix = key.starts_with(L"HASH#");
            if (hasPrefix || IsNumber(key)) {
                try {
                    hash = std::stoul(hasPrefix ? key.substr(5, key.size() - 5) : key);
                }
                catch (...) {
                    return;
                }
            }
            else
                hash = CText::GetHash(WtoA(key).c_str());
            if (!strings.contains(hash))
                strings[hash] = value;
        };
        if (format.first == FILETYPE_XLSX) {
            xlnt::workbook wb;
            wb.load(in.c_str());
            xlnt::worksheet ws = wb.sheet_by_index(0);
            xlnt::row_t last_row = ws.highest_row();
            for (xlnt::row_t row = 2; row <= last_row; ++row) {
                std::wstring valA, valB;
                auto cellA = ws.cell("A" + std::to_string(row));
                auto cellB = ws.cell("B" + std::to_string(row));
                if (cellA.has_value())
                    valA = ToUTF16(cellA.to_string());
                if (cellB.has_value())
                    valB = ToUTF16(cellB.to_string());
                AddKeyAndValue(valA, valB);
            }
            success = true;
        }
        else {
            TextFileTable textFile;
            success = textFile.Read(in, separator == 0 ? fileType[format.first].separator : separator);
            if (success) {
                for (auto const &row : textFile.Rows()) {
                    if (row.size() >= 2)
                        AddKeyAndValue(row[0], row[1]);
                }
            }
            textFile.Clear();
        }
        if (success)
            success = text.LoadTranslationStrings(strings, game);
    }

    ErrorType error = ErrorType::NONE;

    if (!success) {
        ErrorMessage(L"Input file reading error");
        error = ErrorType::INPUT_FILE_READING_ERROR;
    }
    else {
        success = false;
        if (format.second == FILETYPE_HUF)
            success = text.WriteTranslationsFile(out.c_str());
        else {
            std::map<unsigned int, std::wstring> keys;
            if (!keysPath.empty()) {
                TextFileTable keysFile;
                if (keysFile.Read(keysPath)) {
                    for (auto const &row : keysFile.Rows()) {
                        if (!row.empty()) {
                            auto key = row[0];
                            Trim(key);
                            keys[CText::GetHash(WtoA(key).c_str())] = key;
                        }
                    }
                }
                std::set<unsigned int> textHashes;
                for (unsigned int i = 0; i < text.m_nNumStringHashes; ++i)
                    textHashes.insert(text.m_pStringHashes[i].key);
                auto AddTranslationKey = [&textHashes, &keys](std::wstring const &keyName) {
                    unsigned int hash = CText::GetHash(WtoA(keyName).c_str());
                    if (textHashes.contains(hash) && !keys.contains(hash))
                        keys[hash] = keyName;
                };
                for (unsigned int i = 0; i <= 3500; i++) {
                    AddTranslationKey(Format(L"IDS_EA_MAIL_TITLE_%d", i));
                    for (unsigned int v = 0; v < 10; v++)
                        AddTranslationKey(Format(L"IDS_EA_MAIL_TITLE_VAR_%d_%d", v, i));
                    AddTranslationKey(Format(L"IDS_EA_MAIL_TEXT_%d", i));
                    for (unsigned int v = 0; v < 10; v++)
                        AddTranslationKey(Format(L"IDS_EA_MAIL_TEXT_VAR_%d_%d", v, i));
                    AddTranslationKey(Format(L"IDS_EA_MAIL_REMARK_%d", i));
                    for (unsigned int v = 0; v < 3; v++)
                        AddTranslationKey(Format(L"IDS_EA_MAIL_ALT_%d_%d", v, i));
                    for (unsigned int v = 0; v < 3; v++)
                        AddTranslationKey(Format(L"IDS_EA_MAIL_ANSWER_%d_%d", v, i));
                }
                AddTranslationKey(Format(L"IDS_CITYDESC_%08X", 0));
                for (unsigned int countryId = 1; countryId <= 207; countryId++) {
                    for (unsigned int clubIndex = 1; clubIndex <= 0x2100; clubIndex++)
                        AddTranslationKey(Format(L"IDS_CITYDESC_%08X", (countryId << 16) | clubIndex));
                    AddTranslationKey(Format(L"IDS_CITYDESC_%08X", (countryId << 16) | 0xFFFF));
                }
                for (unsigned int i = 0; i < 2000; i++) {
                    for (unsigned int v = 0; v < 20; v++)
                        AddTranslationKey(Format(L"IDS_WEBSITE_%05d_%d", i, v));
                }
                for (unsigned int i = 0; i <= 5000; i++) {
                    for (unsigned int v = 0; v <= 20; v++)
                        AddTranslationKey(Format(L"TM09_%06d_%02d", i, v));
                }
                for (unsigned int i = 0; i <= 5000; i++) {
                    for (unsigned int v = 0; v <= 20; v++)
                        AddTranslationKey(Format(L"TM09LIVE_%06d_%02d", i, v));
                }
            }
            TextFileTable *textFile = nullptr;
            lxw_workbook *excelFile = nullptr;
            lxw_worksheet *excelSheet = nullptr;
            if (format.second == FILETYPE_XLSX) {
                excelFile = workbook_new(ToUTF8(out.c_str()).c_str());
                if (excelFile) {
                    std::string sheetName = ToUTF8(out.stem().c_str()).c_str();
                    excelSheet = workbook_add_worksheet(excelFile, sheetName.empty() ? NULL : sheetName.c_str());
                    if (excelSheet) {
                        lxw_format *textFormat = workbook_add_format(excelFile);
                        if (textFormat) {
                            format_set_num_format(textFormat, "@");
                            worksheet_set_column(excelSheet, 0, 0, 50, textFormat);
                            worksheet_set_column(excelSheet, 1, 1, hashes ? 155 : 160, textFormat);
                            if (hashes)
                                worksheet_set_column(excelSheet, 2, 2, 12, NULL);
                            lxw_table_column col1 = { .header = "Key" };
                            lxw_table_column col2 = { .header = "Text" };
                            lxw_table_column col3 = { .header = "Hash" };
                            lxw_table_column *columns2[] = { &col1, &col2, NULL };
                            lxw_table_column *columns3[] = { &col1, &col2, &col3, NULL };
                            lxw_table_options options = {
                                .style_type = LXW_TABLE_STYLE_TYPE_LIGHT,
                                .style_type_number = 1,
                                .columns = hashes ? columns3 : columns2,
                            };
                            worksheet_add_table(excelSheet, 0, 0, text.m_nNumStringHashes, hashes ? 2 : 1, &options);
                            success = true;
                        }
                    }
                }
            }
            else {
                textFile = new TextFileTable;
                success = textFile != nullptr;
            }
            if (success) {
                if (text.m_pStringHashes && text.m_nNumStringHashes != 0) {
                    unsigned int totalNamed = 0;
                    for (unsigned int i = 0; i < text.m_nNumStringHashes; ++i) {
                        const CStringHash &entry = text.m_pStringHashes[i];
                        const wchar_t *value = text.GetByHashKey(entry.key);
                        if (!value)
                            continue;
                        std::wstring key = std::to_wstring(entry.key);
                        if (format.second == FILETYPE_TR ||!keysPath.empty()) {
                            if (keys.contains(entry.key)) {
                                key = keys[entry.key];
                                totalNamed++;
                            }
                            else
                                key = L"HASH#" + key;
                        }
                        if (excelFile) {
                            worksheet_write_string(excelSheet, i + 1, 0, ToUTF8(key).c_str(), NULL);
                            worksheet_write_string(excelSheet, i + 1, 1, ToUTF8(value).c_str(), NULL);
                            if (hashes)
                                worksheet_write_number(excelSheet, i + 1, 2, entry.key, NULL);
                        }
                        else if (textFile) {
                            if (hashes)
                                textFile->AddRow({ key, value, std::to_wstring(entry.key) });
                            else
                                textFile->AddRow({ key, value });
                        }
                    }
                    if (stats) {
                        ::Message(Format(L"Total named: %d/%d (%.2f%%)", totalNamed, text.m_nNumStringHashes,
                            (float)totalNamed / (float)text.m_nNumStringHashes * 100.0f));
                    }
                }
            }
            if (excelFile)
                workbook_close(excelFile);
            if (textFile) {
                success = textFile->Write(out, separator == 0 ? fileType[format.second].separator : separator, fileType[format.second].encoding);
                delete textFile;
            }
        }
        if (!success) {
            ErrorMessage(L"Output file writing error");
            error = ErrorType::OUTPUT_FILE_WRITING_ERROR;
        }
    }

    text.Clear();

    return error;
}

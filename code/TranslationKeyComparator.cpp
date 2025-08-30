#include "TranslationKeyComparator.h"

TranslationKey::TranslationKey(std::wstring const &_name, CStringHash *_hash) {
    name = _name;
    hash = _hash;
    category = KEYCAT_NONE;
    subcategory = KEYSUBCAT_NONE;
    id1 = 0;
    id2 = 0;
    subid = 0;
    unsigned int i1 = 0, i2 = 0;

    if (name.starts_with(L"HASH#")) {
        if (swscanf(name.c_str(), L"HASH#%d", &i1) == 1) {
            category = KEYCAT_HASH;
            id1 = i1;
        }
    }
    else if (name.starts_with(L"IDS_EA_MAIL_")) {
        if (name.starts_with(L"IDS_EA_MAIL_TITLE_VAR_")) {
            if (swscanf(name.c_str(), L"IDS_EA_MAIL_TITLE_VAR_%d_%d", &i2, &i1) == 2) {
                category = KEYCAT_MAIL;
                subcategory = KEYSUBCAT_TITLE_VAR;
                id1 = i1;
                id2 = i2;
            }
        }
        if (name.starts_with(L"IDS_EA_MAIL_TITLE_")) {
            if (swscanf(name.c_str(), L"IDS_EA_MAIL_TITLE_%d", &i1) == 1) {
                category = KEYCAT_MAIL;
                subcategory = KEYSUBCAT_TITLE;
                id1 = i1;
            }
        }
        else if (name.starts_with(L"IDS_EA_MAIL_TEXT_VAR_")) {
            if (swscanf(name.c_str(), L"IDS_EA_MAIL_TEXT_VAR_%d_%d", &i2, &i1) == 2) {
                category = KEYCAT_MAIL;
                subcategory = KEYSUBCAT_TEXT_VAR;
                id1 = i1;
                id2 = i2;
            }
        }
        else if (name.starts_with(L"IDS_EA_MAIL_TEXT_")) {
            if (swscanf(name.c_str(), L"IDS_EA_MAIL_TEXT_%d", &i1) == 1) {
                category = KEYCAT_MAIL;
                subcategory = KEYSUBCAT_TEXT;
                id1 = i1;
            }
        }
        else if (name.starts_with(L"IDS_EA_MAIL_REMARK_")) {
            if (swscanf(name.c_str(), L"IDS_EA_MAIL_REMARK_%d", &i1) == 1) {
                category = KEYCAT_MAIL;
                subcategory = KEYSUBCAT_REMARK;
                id1 = i1;
            }
        }
        else if (name.starts_with(L"IDS_EA_MAIL_ALT_")) {
            if (swscanf(name.c_str(), L"IDS_EA_MAIL_ALT_%d_%d", &i2, &i1) == 2) {
                category = KEYCAT_MAIL;
                subcategory = KEYSUBCAT_ALT;
                id1 = i1;
                subid = i2;
            }
        }
        else if (name.starts_with(L"IDS_EA_MAIL_ANSWER_")) {
            if (swscanf(name.c_str(), L"IDS_EA_MAIL_ANSWER_%d_%d", &i2, &i1) == 2) {
                category = KEYCAT_MAIL;
                subcategory = KEYSUBCAT_ANSWER;
                id1 = i1;
                subid = i2;
            }
        }
    }
    else if (name.starts_with(L"TM09_")) {
        if (swscanf(name.c_str(), L"TM09_%06d_%02d", &i1, &i2) == 2) {
            category = KEYCAT_TM09;
            subcategory = KEYSUBCAT_TITLE;
            id1 = i1;
            id2 = i2;
        }
    }
    else if (name.starts_with(L"TM09LIVE_")) {
        if (swscanf(name.c_str(), L"TM09LIVE_%06d_%02d", &i1, &i2) == 2) {
            category = KEYCAT_TM09;
            subcategory = KEYSUBCAT_TEXT;
            id1 = i1;
            id2 = i2;
        }
    }
    else if (name.starts_with(L"IDS_HELP_")) {
        wchar_t screenName[256] = {};
        wchar_t categoryName[256] = {};
        if (swscanf(name.c_str(), L"IDS_HELP_%s_%s_%d", screenName, categoryName, &i1) == 3) {
            std::wstring categoryName_ = categoryName;
            if (categoryName_ == L"HEADLINE") {
                category = KEYCAT_HELP;
                subcategory = KEYSUBCAT_TITLE;
                id1 = i1;
                stringId = screenName;
            }
            else if (categoryName_ == L"INFO") {
                category = KEYCAT_HELP;
                subcategory = KEYSUBCAT_TEXT;
                id1 = i1;
                stringId = screenName;
            }
        }
    }
    else if (name.starts_with(L"ACHIEVEMENT_HEADER_")) {
        if (swscanf(name.c_str(), L"ACHIEVEMENT_HEADER_%d", &i1) == 1) {
            category = KEYCAT_ACHIEVEMENT;
            subcategory = KEYSUBCAT_TITLE;
            id1 = i1;
        }
    }
    else if (name.starts_with(L"ACHIEVEMENT_TEXT_")) {
        if (swscanf(name.c_str(), L"ACHIEVEMENT_TEXT_%d", &i1) == 1) {
            category = KEYCAT_ACHIEVEMENT;
            subcategory = KEYSUBCAT_TEXT;
            id1 = i1;
        }
    }
    else if (name.starts_with(L"REWARD_HEADER_")) {
        if (swscanf(name.c_str(), L"REWARD_HEADER_%d", &i1) == 1) {
            category = KEYCAT_REWARD;
            subcategory = KEYSUBCAT_TITLE;
            id1 = i1;
        }
    }
    else if (name.starts_with(L"REWARD_TEXT_")) {
        if (swscanf(name.c_str(), L"REWARD_TEXT_%d", &i1) == 1) {
            category = KEYCAT_REWARD;
            subcategory = KEYSUBCAT_TEXT;
            id1 = i1;
        }
    }
    else if (name.starts_with(L"IDS_CHARACTER_FULL_")) {
        if (swscanf(name.c_str(), L"IDS_CHARACTER_FULL_%d", &i1) == 1) {
            category = KEYCAT_CHARACTER;
            subcategory = KEYSUBCAT_TITLE;
            id1 = i1;
        }
    }
    else if (name.starts_with(L"IDS_CHARACTER_ABBR_")) {
        if (swscanf(name.c_str(), L"IDS_CHARACTER_ABBR_%d", &i1) == 1) {
            category = KEYCAT_CHARACTER;
            subcategory = KEYSUBCAT_TEXT;
            id1 = i1;
        }
    }
    else if (name.starts_with(L"IDS_3DMATCH_HINT")) {
        wchar_t categoryName[256] = {};
        if (swscanf(name.c_str(), L"IDS_3DMATCH_HINT%d_%s", &i1, categoryName) == 3) {
            std::wstring categoryName_ = categoryName;
            if (categoryName_ == L"TITLE") {
                category = KEYCAT_3DMATCHHINT;
                subcategory = KEYSUBCAT_TITLE;
                id1 = i1;
            }
            else if (categoryName_ == L"TEXT") {
                category = KEYCAT_3DMATCHHINT;
                subcategory = KEYSUBCAT_TEXT;
                id1 = i1;
            }
        }
    }
}

unsigned long TranslationKeyComparator::ParseNumber(const std::wstring &s) {
    if (s.size() == 8 && s[0] == '0' && std::all_of(s.begin(), s.end(), ::isxdigit))
        return std::stoul(s, nullptr, 16);
    return std::stoul(s, nullptr, 10);
}

int TranslationKeyComparator::NaturalCompare(const std::wstring &a, const std::wstring &b) {
    size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (iswdigit(a[i]) && iswdigit(b[j])) {
            size_t ia = i, ib = j;
            while (i < a.size() && iswdigit(a[i])) i++;
            while (j < b.size() && iswdigit(b[j])) j++;
            auto numA = ParseNumber(a.substr(ia, i - ia));
            auto numB = ParseNumber(b.substr(ib, j - ib));
            if (numA != numB) return numA < numB ? -1 : 1;
        }
        else {
            if (a[i] != b[j]) return a[i] < b[j] ? -1 : 1;
            i++; j++;
        }
    }
    return (a.size() == b.size()) ? 0 : (a.size() < b.size() ? -1 : 1);
}

bool TranslationKeyComparator::Compare(const TranslationKey &a, const TranslationKey &b) {
    if (a.category != 0 && a.category == b.category) {
        if (a.stringId < b.stringId) return true;
        if (b.stringId < a.stringId) return false;
        if (a.id1 < b.id1) return true;
        if (b.id1 < a.id1) return false;
        if (a.id2 < b.id2) return true;
        if (b.id2 < a.id2) return false;
        if (a.subcategory < b.subcategory) return true;
        if (b.subcategory < a.subcategory) return false;
        return a.subid < b.subid;
    }
    return NaturalCompare(a.name, b.name) < 0;
}

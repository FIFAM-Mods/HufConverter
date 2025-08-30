#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <regex>
#include <unordered_map>
#include "Text.h"

enum eKeyCategory {
    KEYCAT_NONE = 0,
    KEYCAT_HASH,
    KEYCAT_MAIL,
    KEYCAT_TM09,
    KEYCAT_HELP,
    KEYCAT_ACHIEVEMENT,
    KEYCAT_REWARD,
    KEYCAT_CHARACTER,
    KEYCAT_3DMATCHHINT
};

enum eKeySubCategory {
    KEYSUBCAT_NONE = 0,
    KEYSUBCAT_TITLE,
    KEYSUBCAT_TITLE_VAR,
    KEYSUBCAT_TEXT,
    KEYSUBCAT_TEXT_VAR,
    KEYSUBCAT_REMARK,
    KEYSUBCAT_ALT,
    KEYSUBCAT_ANSWER
};

class TranslationKey {
public:
    std::wstring name;
    CStringHash *hash;
    eKeyCategory category;
    eKeySubCategory subcategory;
    unsigned int id1;
    unsigned int id2;
    unsigned int subid;
    std::wstring stringId;

    TranslationKey(std::wstring const &_name, CStringHash *_hash);
};

class TranslationKeyComparator {
    static unsigned long ParseNumber(const std::wstring &s);
    static int NaturalCompare(const std::wstring &a, const std::wstring &b);
public:
    static bool Compare(const TranslationKey &a, const TranslationKey &b);
};

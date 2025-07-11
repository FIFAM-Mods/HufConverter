#include "Text.h"
#include <list>
#include <algorithm>
#include "utils.h"
#include "message.h"

CTextMultibyteStrings::~CTextMultibyteStrings() {
    Clear();
}

void CTextMultibyteStrings::Clear() {
    delete[] m_pData;
    m_pData = nullptr;
    m_bitOffset = 0;
    m_nDataSize = 0;
    m_nRuntimeDataPtr = 0;
    m_nTotalLength = 0;
}

bool CTextMultibyteStrings::Read(HANDLE fileHandle) {
    Clear();
    if (fileHandle == INVALID_HANDLE_VALUE)
        return false;
    DWORD bytesRead = 0;
    unsigned int magic = 0, sectionSize = 0;
    if (!ReadFile(fileHandle, &magic, 4, &bytesRead, nullptr) || magic != 'MBST' ||
        !ReadFile(fileHandle, &m_bitOffset, 4, &bytesRead, nullptr) ||
        !ReadFile(fileHandle, &sectionSize, 4, &bytesRead, nullptr) ||
        sectionSize >= 0x4000000)
    {
        return false;
    }
    m_pData = new unsigned char[sectionSize];
    if (!m_pData || !ReadFile(fileHandle, m_pData, sectionSize, &bytesRead, nullptr)) {
        Clear();
        return false;
    }
    m_nDataSize = sectionSize;
    return true;
}

bool CTextMultibyteStrings::Write(HANDLE fileHandle) const {
    if (fileHandle == INVALID_HANDLE_VALUE)
        return false;
    DWORD written = 0;
    unsigned int magic = 'MBST';
    unsigned int byteSize = (m_bitOffset / 8) + 1;
    return WriteFile(fileHandle, &magic, 4, &written, nullptr) &&
        WriteFile(fileHandle, &m_bitOffset, 4, &written, nullptr) &&
        WriteFile(fileHandle, &byteSize, 4, &written, nullptr) &&
        WriteFile(fileHandle, m_pData, byteSize, &written, nullptr);
}

unsigned char CTextMultibyteStrings::GetBitAt(unsigned int offset) const {
    unsigned int byteIndex = offset / 8;
    unsigned int bitIndex = offset % 8;
    if (byteIndex >= m_nDataSize)
        return 0;
    return (m_pData[byteIndex] >> bitIndex) & 1;
}

bool CTextMultibyteStrings::WriteBits(unsigned int value, unsigned char numBits) {
    if (numBits == 0 || numBits > 32)
        return false;
    unsigned int requiredBits = m_bitOffset + numBits;
    unsigned int requiredBytes = (requiredBits + 7) / 8;
    if (requiredBytes > m_nDataSize) {
        unsigned int newSize = m_nDataSize + 0x100000; // +1MB
        unsigned char *newData = new unsigned char[newSize]();
        if (!newData)
            return false;
        if (m_pData) {
            memcpy(newData, m_pData, m_nDataSize);
            delete[] m_pData;
        }
        m_pData = newData;
        m_nDataSize = newSize;
    }
    unsigned int byteIndex = m_bitOffset / 8;
    unsigned int bitIndex = m_bitOffset % 8;
    for (int i = numBits - 1; i >= 0; --i) {
        unsigned char bit = (value >> i) & 1;
        m_pData[byteIndex] |= bit << bitIndex;
        if (++bitIndex == 8) {
            bitIndex = 0;
            ++byteIndex;
        }
    }
    m_bitOffset = byteIndex * 8 + bitIndex;
    return true;
}

CHuffChunk::CHuffChunk() {
    frequency = 0;
    character = 0;
    ParentLeaf = 0xFFFF;
    RightLeaf = 0xFFFF;
    LeftLeaf = 0xFFFF;
}

CHuffChunk::CHuffChunk(wchar_t Character, unsigned int Frequency) {
    frequency = Frequency;
    character = Character;
    ParentLeaf = 0xFFFF;
    RightLeaf = 0xFFFF;
    LeftLeaf = 0xFFFF;
}

bool CHuffChunk::IsLast() {
    return RightLeaf == 0xFFFF;
}

CTextHuffman::~CTextHuffman() {
    Clear();
}

void CTextHuffman::Clear() {
    delete[] m_pHuffChunks;
    m_pHuffChunks = nullptr;
    delete[] m_pCharactersInfo;
    m_pCharactersInfo = nullptr;
    m_nNumHuffmanChunks = 0;
    m_nNumUniqueCharacters = 0;
    m_nRootLeaf = 0xFFFF;
}

unsigned short CTextHuffman::GetRootLeaf() {
    return m_nRootLeaf;
}

CHuffChunk *CTextHuffman::GetChunk(unsigned short chunkIndex) {
    if (chunkIndex < m_nNumHuffmanChunks && m_pHuffChunks)
        return &m_pHuffChunks[chunkIndex];
    static CHuffChunk DUMMY_CHUNK(0, 0);
    return &DUMMY_CHUNK;
}

CharacterInfo *CTextHuffman::FindCharacterInfo(wchar_t ch) {
    static CharacterInfo DUMMY_CHARACTER_INFO;
    if (!m_pCharactersInfo)
        return &DUMMY_CHARACTER_INFO;
    auto it = std::find_if(m_pCharactersInfo, &m_pCharactersInfo[m_nNumUniqueCharacters], [ch](CharacterInfo &c) {
        return c.character == ch;
        });
    if (it == &m_pCharactersInfo[m_nNumUniqueCharacters])
        return &DUMMY_CHARACTER_INFO;
    return it;
}

CHuffChunk *CTextHuffman::GetNextLeaf(unsigned short *currLeaf, unsigned char bit) {
    if (bit & 1)
        *currLeaf = GetChunk(*currLeaf)->LeftLeaf;
    else
        *currLeaf = GetChunk(*currLeaf)->RightLeaf;
    return GetChunk(*currLeaf);
}

bool CTextHuffman::Read(HANDLE fileHandle) {
    if (fileHandle == INVALID_HANDLE_VALUE)
        return false;
    Clear();
    DWORD bytesRead = 0;
    unsigned int magic = 0;
    if (!ReadFile(fileHandle, &magic, sizeof(magic), &bytesRead, nullptr) || magic != 'HUFF')
        return false;
    if (!ReadFile(fileHandle, &m_nNumHuffmanChunks, 2, &bytesRead, nullptr) ||
        !ReadFile(fileHandle, &m_nRootLeaf, 2, &bytesRead, nullptr) ||
        !ReadFile(fileHandle, &m_nNumUniqueCharacters, 2, &bytesRead, nullptr))
    {
        return false;
    }
    m_pHuffChunks = new CHuffChunk[m_nNumHuffmanChunks];
    m_pCharactersInfo = new CharacterInfo[m_nNumUniqueCharacters];
    if (!ReadFile(fileHandle, m_pHuffChunks, m_nNumHuffmanChunks * 12, &bytesRead, nullptr) ||
        !ReadFile(fileHandle, m_pCharactersInfo, m_nNumUniqueCharacters * 8, &bytesRead, nullptr))
    {
        Clear();
        return false;
    }
    return true;
}

bool CTextHuffman::Write(HANDLE hFile) {
    if (hFile == INVALID_HANDLE_VALUE)
        return false;
    DWORD written = 0;
    unsigned int magic = 'HUFF';
    return WriteFile(hFile, &magic, 4, &written, nullptr) &&
        WriteFile(hFile, &m_nNumHuffmanChunks, 2, &written, nullptr) &&
        WriteFile(hFile, &m_nRootLeaf, 2, &written, nullptr) &&
        WriteFile(hFile, &m_nNumUniqueCharacters, 2, &written, nullptr) &&
        WriteFile(hFile, m_pHuffChunks, 12 * m_nNumHuffmanChunks, &written, nullptr) &&
        WriteFile(hFile, m_pCharactersInfo, 8 * m_nNumUniqueCharacters, &written, nullptr);
}

void CTextHuffman::GenerateHuffmanCodes(unsigned short nodeIndex, unsigned short &outIndex) {
    CHuffChunk *chunk = GetChunk(nodeIndex);
    if (chunk->IsLast() && m_pCharactersInfo && outIndex < m_nNumUniqueCharacters) {
        CharacterInfo &info = m_pCharactersInfo[outIndex];
        info.character = chunk->character;
        info.codeBits = 0;
        info.codeLength = 0;
        unsigned short current = nodeIndex;
        while (current != 0xFFFF) {
            unsigned short parent = GetChunk(current)->ParentLeaf;
            if (parent != 0xFFFF) {
                CHuffChunk *parentChunk = GetChunk(parent);
                if (parentChunk->LeftLeaf == current)
                    info.codeBits |= (1 << info.codeLength);
                ++info.codeLength;
            }
            current = parent;
        }
        outIndex++;
    }
    else {
        if (chunk->RightLeaf != 0xFFFF)
            GenerateHuffmanCodes(chunk->RightLeaf, outIndex);
        if (chunk->LeftLeaf != 0xFFFF)
            GenerateHuffmanCodes(chunk->LeftLeaf, outIndex);
    }
}

bool CTextHuffman::Pack(unsigned int *characterMap) {
    Clear();
    if (!characterMap)
        return false;
    for (unsigned int c = 0; c < 65536; c++) {
        if (characterMap[c])
            m_nNumUniqueCharacters++;
    }
    if (m_nNumUniqueCharacters < 2 || m_nNumUniqueCharacters >= 32768)
        return false;
    m_nNumHuffmanChunks = m_nNumUniqueCharacters;
    m_pHuffChunks = new CHuffChunk[m_nNumUniqueCharacters * 2];
    std::list<unsigned short> huffList;
    unsigned int leafNumber = 0;
    for (unsigned int c = 0; c < 65536; c++) {
        if (characterMap[c]) {
            m_pHuffChunks[leafNumber] = CHuffChunk(c, characterMap[c]);
            huffList.push_back(leafNumber);
            leafNumber++;
        }
    }
    huffList.sort([&](unsigned short a, unsigned short b) {
        return m_pHuffChunks[a].frequency < m_pHuffChunks[b].frequency;
        });
    while (huffList.size() > 1) {
        CHuffChunk &parent = m_pHuffChunks[m_nNumHuffmanChunks];
        parent.character = 0;
        parent.ParentLeaf = 0xFFFF;
        unsigned short rightIdx = huffList.front(); huffList.pop_front();
        unsigned short leftIdx = huffList.front(); huffList.pop_front();
        parent.RightLeaf = rightIdx;
        parent.LeftLeaf = leftIdx;
        CHuffChunk *right = GetChunk(rightIdx);
        CHuffChunk *left = GetChunk(leftIdx);
        parent.frequency = right->frequency + left->frequency;
        right->ParentLeaf = m_nNumHuffmanChunks;
        left->ParentLeaf = m_nNumHuffmanChunks;
        auto insertIt = huffList.begin();
        while (insertIt != huffList.end()) {
            CHuffChunk &node = m_pHuffChunks[*insertIt];
            if (node.frequency > parent.frequency)
                break;
            ++insertIt;
        }
        huffList.insert(insertIt, m_nNumHuffmanChunks);
        ++m_nNumHuffmanChunks;
    }
    if (m_nNumHuffmanChunks < 2 * m_nNumUniqueCharacters) {
        CHuffChunk *newChunks = new CHuffChunk[m_nNumHuffmanChunks];
        memcpy(newChunks, m_pHuffChunks, m_nNumHuffmanChunks * sizeof(CHuffChunk));
        delete[] m_pHuffChunks;
        m_pHuffChunks = newChunks;
    }
    m_nRootLeaf = huffList.front();
    huffList.clear();
    m_pCharactersInfo = new CharacterInfo[m_nNumUniqueCharacters];
    unsigned short nextCharacterIndex = 0;
    GenerateHuffmanCodes(m_nRootLeaf, nextCharacterIndex);
    std::sort(m_pCharactersInfo, m_pCharactersInfo + m_nNumUniqueCharacters, [](const CharacterInfo &a, const CharacterInfo &b) {
        return a.character < b.character;
    });
    return true;
}

CText::~CText() {
    Clear();
}

unsigned int CText::GetHash(const char *str) {
    if (!str)
        return 0;
    unsigned int result = 0;
    while (*str) {
        result = (result << 16) + (result << 6) + static_cast<unsigned char>(*str) - result;
        ++str;
    }
    return result;
}

void CText::AllocateTempStrings() {
    if (m_nMaxStringLength) {
        for (int i = 0; i < 32; i++) {
            delete[] m_pTempStrings[i];
            m_pTempStrings[i] = new wchar_t[m_nMaxStringLength + 1]();
        }
    }
}

void CText::Clear() {
    delete[] m_pStringHashes;
    m_pStringHashes = nullptr;
    for (int i = 0; i < 32; i++) {
        delete[] m_pTempStrings[i];
        m_pTempStrings[i] = nullptr;
    }
    m_game = GAME_NOTSET;
    m_nMaxStringLength = 0;
    m_nNumStringHashes = 0;
    m_nTempStringsCounter = 0;
    m_mbStrings.Clear();
    m_huffmanInfo.Clear();
}


unsigned __int64 GetBytesLeftToRead(HANDLE file) {
    LARGE_INTEGER fileSize;
    LARGE_INTEGER currentPos;
    GetFileSizeEx(file, &fileSize);
    LARGE_INTEGER zero = { 0 };
    SetFilePointerEx(file, zero, &currentPos, FILE_CURRENT);
    return fileSize.QuadPart - currentPos.QuadPart;
}

bool CText::LoadTranslationsFile(wchar_t const *filePath, eGame game) {
    if (!filePath || !*filePath)
        return false;
    Clear();
    m_game = game;
    DWORD bytesRead = 0;
    HANDLE file = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;
    bool success = false;
    if (game == GAME_FM09) {
        unsigned int magic = 0;
        unsigned int version = 0;
        if (ReadFile(file, &magic, 4, &bytesRead, nullptr) && magic == 'BFLC' &&
            ReadFile(file, &version, 4, &bytesRead, nullptr) && version <= 1 &&
            ReadFile(file, &m_nLanguageID, 4, &bytesRead, nullptr) &&
            ReadFile(file, &m_nMaxStringLength, 4, &bytesRead, nullptr) &&
            m_huffmanInfo.Read(file) &&
            m_mbStrings.Read(file) &&
            ReadFile(file, &m_nNumStringHashes, 4, &bytesRead, nullptr))
        {
            m_pStringHashes = new CStringHash[m_nNumStringHashes];
            success = ReadFile(file, m_pStringHashes, 8 * m_nNumStringHashes, &bytesRead, nullptr);
        }
    }
    else {
        if (ReadFile(file, &m_nMaxStringLength, 4, &bytesRead, nullptr) &&
            ReadFile(file, &m_nNumStringHashes, 4, &bytesRead, nullptr))
        {
            m_nMaxStringLength -= 1;
            m_pStringHashes = new CStringHash[m_nNumStringHashes];
            if (ReadFile(file, m_pStringHashes, 8 * m_nNumStringHashes, &bytesRead, nullptr) &&
                ReadFile(file, &m_huffmanInfo.m_nNumHuffmanChunks, 2, &bytesRead, nullptr) &&
                ReadFile(file, &m_huffmanInfo.m_nRootLeaf, 2, &bytesRead, nullptr))
            {
                unsigned int NodeArraySize = (game == GAME_TCM2005) ? 256 : 512;
                unsigned int MbStringsSizeInBits = 0;
                CHuffChunk *huffChunks = new CHuffChunk[NodeArraySize];
                if (ReadFile(file, huffChunks, NodeArraySize * 12, &bytesRead, nullptr) &&
                    ReadFile(file, &m_mbStrings.m_bitOffset, 4, &bytesRead, nullptr) &&
                    ReadFile(file, &MbStringsSizeInBits, 4, &bytesRead, nullptr) &&
                    ReadFile(file, &m_mbStrings.m_nRuntimeDataPtr, 4, &bytesRead, nullptr))
                {
                    m_mbStrings.m_nDataSize = (unsigned int)GetBytesLeftToRead(file);
                    m_mbStrings.m_nTotalLength = (MbStringsSizeInBits + 7) / 8 + ((m_game == GAME_TCM2005) ? 0xC10 : 0x1810);
                    m_huffmanInfo.m_pHuffChunks = new CHuffChunk[m_huffmanInfo.m_nNumHuffmanChunks];
                    memcpy(m_huffmanInfo.m_pHuffChunks, huffChunks, m_huffmanInfo.m_nNumHuffmanChunks * 12);
                    for (unsigned int i = 0; i < m_huffmanInfo.m_nNumHuffmanChunks; i++)
                        m_huffmanInfo.m_pHuffChunks[i].character &= 0xFF;
                    m_mbStrings.m_pData = new unsigned char[m_mbStrings.m_nDataSize];
                    success = m_mbStrings.m_pData && ReadFile(file, m_mbStrings.m_pData, m_mbStrings.m_nDataSize, &bytesRead, nullptr);
                }
                delete[] huffChunks;
            }
        }
    }
    CloseHandle(file);
    if (!success)
        Clear();
    else
        AllocateTempStrings();
    return success;
}

bool CText::WriteTranslationsFile(wchar_t const *filePath) {
    if (!filePath || !*filePath)
        return false;
    HANDLE file = CreateFileW(filePath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return false;
    DWORD written = 0;
    bool success = false;
    if (m_game == GAME_FM09) {
        unsigned int magic = 'BFLC';
        unsigned int version = 1;
        if (WriteFile(file, &magic, 4, &written, nullptr) &&
            WriteFile(file, &version, 4, &written, nullptr) &&
            WriteFile(file, &m_nLanguageID, 4, &written, nullptr) &&
            WriteFile(file, &m_nMaxStringLength, 4, &written, nullptr) &&
            m_huffmanInfo.Write(file) &&
            m_mbStrings.Write(file) &&
            WriteFile(file, &m_nNumStringHashes, 4, &written, nullptr) &&
            WriteFile(file, m_pStringHashes, m_nNumStringHashes * sizeof(CStringHash), &written, nullptr))
        {
            success = true;
        }
    }
    else {
        unsigned int maxLengthWithTerminator = m_nMaxStringLength + 1;
        if (WriteFile(file, &maxLengthWithTerminator, 4, &written, nullptr) &&
            WriteFile(file, &m_nNumStringHashes, 4, &written, nullptr) &&
            WriteFile(file, m_pStringHashes, 8 * m_nNumStringHashes, &written, nullptr) &&
            WriteFile(file, &m_huffmanInfo.m_nNumHuffmanChunks, 2, &written, nullptr) &&
            WriteFile(file, &m_huffmanInfo.m_nRootLeaf, 2, &written, nullptr))
        {
            unsigned int NodeArraySize = (m_game == GAME_TCM2005) ? 256 : 512;
            CHuffChunk *huffChunks = new CHuffChunk[NodeArraySize];
            memset(huffChunks, 0, NodeArraySize * 12);
            memcpy(huffChunks, m_huffmanInfo.m_pHuffChunks, m_huffmanInfo.m_nNumHuffmanChunks * 12);
            unsigned int MbStringsSizeInBits = (m_mbStrings.m_nTotalLength - ((m_game == GAME_TCM2005) ? 0xC10 : 0x1810)) * 8;
            unsigned int byteSize = (m_mbStrings.m_bitOffset / 8) + 1;
            if (WriteFile(file, huffChunks, NodeArraySize * 12, &written, nullptr) &&
                WriteFile(file, &MbStringsSizeInBits, 4, &written, nullptr) &&
                WriteFile(file, &m_mbStrings.m_bitOffset, 4, &written, nullptr) &&
                WriteFile(file, &m_mbStrings.m_nRuntimeDataPtr, 4, &written, nullptr) &&
                WriteFile(file, m_mbStrings.m_pData, byteSize, &written, nullptr))
            {
                success = true;
            }
            delete[] huffChunks;
        }
    }
    CloseHandle(file);
    return success;
}

bool CText::IsKeyPresent(char const *key) {
    return GetByHashKey(GetHash(key)) != 0;
}

wchar_t const *CText::Get(char const *key) {
    return GetByKeyName(key);
}

wchar_t const *CText::GetByKeyName(char const *key) {
    if (!key)
        return nullptr;
    wchar_t const *str = GetByHashKey(GetHash(key));
    if (!str) {
        static wchar_t buf[4096];
        buf[0] = L'\0';
        wcscpy(buf, AtoW(key).c_str());
        return buf;
    }
    return str;
}

wchar_t const *CText::GetByHashKey(unsigned int hashKey) {
    if (!m_pStringHashes || !m_nMaxStringLength)
        return nullptr;
    auto it = std::find_if(m_pStringHashes, &m_pStringHashes[m_nNumStringHashes], [hashKey](CStringHash &s) {
        return s.key == hashKey;
        });
    if (it == &m_pStringHashes[m_nNumStringHashes])
        return nullptr;
    wchar_t *outStr = m_pTempStrings[m_nTempStringsCounter++];
    if (m_nTempStringsCounter == 32)
        m_nTempStringsCounter = 0;
    wchar_t *pStr = outStr;
    unsigned int bitOffset = it->offset;
    for (unsigned int i = 0; i < m_nMaxStringLength; i++) {
        unsigned short leaf = m_huffmanInfo.GetRootLeaf();
        while (true) {
            unsigned char bit = m_mbStrings.GetBitAt(bitOffset++);
            CHuffChunk *chunk = m_huffmanInfo.GetNextLeaf(&leaf, bit);
            if (chunk->IsLast()) {
                *pStr = chunk->character;
                break;
            }
        }
        if (*pStr == 0)
            break;
        ++pStr;
    }
    return outStr;
}

bool CText::EncodeString(wchar_t const *str) {
    if (!str)
        return false;
    while (true) {
        CharacterInfo *info = m_huffmanInfo.FindCharacterInfo(*str);
        if (!info || info->codeLength == 0)
            return false;
        if (!m_mbStrings.WriteBits(info->codeBits, info->codeLength))
            return false;
        m_mbStrings.m_nTotalLength++;
        if (*str == 0)
            break;
        str++;
    }
    return true;
}

bool CText::LoadTranslationStrings(std::map<unsigned int, std::wstring> const &strings, eGame game) {
    Clear();
    m_game = game;
    static unsigned int characterMap[65536];
    memset(characterMap, 0, sizeof(characterMap));
    for (auto const &[key, str] : strings) {
        for (wchar_t ch : str)
            characterMap[ch]++;
        characterMap[0]++;
    }
    m_huffmanInfo.Pack(characterMap);
    if (m_game != GAME_FM09) {
        unsigned int NodeArraySize = (m_game == GAME_TCM2005) ? 256 : 512;
        if (m_huffmanInfo.m_nNumHuffmanChunks > NodeArraySize) {
            ErrorMessage(Format(L"Reached Huffman Nodes array limit: %d nodes are generated (%d max)",
                m_huffmanInfo.m_nNumHuffmanChunks, NodeArraySize));
            Clear();
            return false;
        }
    }
    m_nNumStringHashes = strings.size();
    m_pStringHashes = new CStringHash[m_nNumStringHashes];
    unsigned int stringCounter = 0;
    for (auto const &[key, str] : strings) {
        m_pStringHashes[stringCounter].key = key;
        m_pStringHashes[stringCounter].offset = m_mbStrings.m_bitOffset;
        EncodeString(str.c_str());
        m_nMaxStringLength = max(m_nMaxStringLength, str.size());
        stringCounter++;
    }
    m_mbStrings.m_nRuntimeDataPtr = (unsigned int)m_mbStrings.m_pData;
    AllocateTempStrings();
    return true;
}

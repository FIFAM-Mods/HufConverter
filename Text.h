#pragma once
#include <stdio.h>
#include <Windows.h>
#include <algorithm>
#include "utils.h"
#include <map>
#include <list>

enum eGame {
    GAME_NOTSET,
    GAME_TCM2005, // 256 tree nodes, Latin-1
    GAME_FM06, // 512 tree nodes, Latin-1
    GAME_FM09 // unlimited tree nodes, UTF-16
};

struct CharacterInfo {
    unsigned int codeBits;
    wchar_t character;
    unsigned char codeLength;
    char _pad7;
};

class CTextMultibyteStrings {
public:
    unsigned char *m_pData = nullptr;
    unsigned int m_bitOffset = 0;
    unsigned int m_nDataSize = 0;
    unsigned int m_nRuntimeDataPtr = 0;
    unsigned int m_nTotalLength = 0;

    ~CTextMultibyteStrings();
    void Clear();
    bool Read(HANDLE fileHandle);
    bool Write(HANDLE fileHandle) const;
    unsigned char GetBitAt(unsigned int offset) const;
    bool WriteBits(unsigned int value, unsigned char numBits);
};

class CHuffChunk {
public:
    unsigned int frequency = 0;
    wchar_t character = 0;
    unsigned short ParentLeaf = 0xFFFF;
    unsigned short RightLeaf = 0xFFFF;
    unsigned short LeftLeaf = 0xFFFF;

    CHuffChunk();
    CHuffChunk(wchar_t Character, unsigned int Frequency);
    bool IsLast();
};

class CTextHuffman {
public:
    CHuffChunk *m_pHuffChunks = nullptr;
    CharacterInfo *m_pCharactersInfo = nullptr;
    unsigned short m_nRootLeaf = 0xFFFF;
    unsigned short m_nNumHuffmanChunks = 0;
    unsigned short m_nNumUniqueCharacters = 0;

    ~CTextHuffman();
    void Clear();
    unsigned short GetRootLeaf();
    CHuffChunk *GetChunk(unsigned short chunkIndex);
    CharacterInfo *FindCharacterInfo(wchar_t ch);
    CHuffChunk *GetNextLeaf(unsigned short *currLeaf, unsigned char bit);
    bool Read(HANDLE fileHandle);
    bool Write(HANDLE hFile);
    void GenerateHuffmanCodes(unsigned short nodeIndex, unsigned short &outIndex);
    bool Pack(unsigned int *characterMap);
};

class CStringHash {
public:
    unsigned int key = 0;
    unsigned int offset = 0;
};

class CText {
public:
    eGame m_game = GAME_NOTSET;
    unsigned int m_nLanguageID = 0;
    CTextMultibyteStrings m_mbStrings;
    CTextHuffman m_huffmanInfo;
    CStringHash *m_pStringHashes = nullptr;
    unsigned int m_nMaxStringLength = 0;
    unsigned int m_nNumStringHashes = 0;
    wchar_t *m_pTempStrings[32] = {};
    unsigned int m_nTempStringsCounter = 0;

    ~CText();
    static unsigned int GetHash(const char *str);
    void AllocateTempStrings();
    void Clear();
    bool LoadTranslationsFile(wchar_t const *filePath, eGame game);
    bool WriteTranslationsFile(wchar_t const *filePath);
    bool IsKeyPresent(char const *key);
    wchar_t const *Get(char const *key);
    wchar_t const *GetByKeyName(char const *key);
    wchar_t const *GetByHashKey(unsigned int hashKey);
    bool EncodeString(wchar_t const *str);
    bool LoadTranslationStrings(std::map<unsigned int, std::wstring> const &strings, eGame game);
};

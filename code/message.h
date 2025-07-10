#pragma once
#include <string>

enum MessageDisplayType {
    MSG_MESSAGE_BOX,
    MSG_CONSOLE,
    MSG_NONE
};

extern MessageDisplayType displayType;

void SetMessageDisplayType(MessageDisplayType type);
bool ErrorMessage(std::wstring const &msg);
bool InfoMessage(std::wstring const &msg);

#include "message.h"
#include "utils.h"
#include <iostream>

MessageDisplayType displayType = MessageDisplayType::MSG_NONE;

void SetMessageDisplayType(MessageDisplayType type) {
    displayType = type;
}

void Message(std::wstring const &msg, bool error) {
    if (displayType == MessageDisplayType::MSG_MESSAGE_BOX) {
        Error(msg.c_str());
    }
    else if (displayType == MessageDisplayType::MSG_CONSOLE)
        std::wcout << msg << std::endl;
}

bool ErrorMessage(std::wstring const &msg) {
    Message(msg, true);
    return false;
}

bool InfoMessage(std::wstring const &msg) {
    Message(msg, false);
    return true;
}

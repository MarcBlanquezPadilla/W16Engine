#include "Log.h"
#include <cstdarg>
#include <cstdio>
#include <iostream>

void Log(LogType type, const char file[], int line, const char* format, ...)
{
    const int bufferSize = 2048;
    char textBuffer[bufferSize];

    va_list args;
    va_start(args, format);
    vsnprintf(textBuffer, bufferSize, format, args);
    va_end(args);
    
    std::string header = "";
    switch (type) {
    case LogType::LOG_INFO:    header = "[INFO] "; break;
    case LogType::LOG_WARNING: header = "[WARN] "; break;
    case LogType::LOG_ERROR:   header = "[ERROR] "; break;
    }

    std::string message = header + textBuffer;

    LogBuffer::GetInstance().AddLog(type, message);

    printf("%s\n", message.c_str());
}
#pragma once
#ifndef __LOG_H__
#define __LOG_H__

#include <cstdio>
#include <cstdarg>
#include <string>
#include <vector>

class LogBuffer
{
public:
    static LogBuffer& GetInstance() {
        static LogBuffer instance;
        return instance;
    }

    void AddMessage(const std::string& msg) {
        messages.push_back(msg);

        if (messages.size() > 500) {
            messages.erase(messages.begin());
        }
    }

    const std::vector<std::string>& GetMessages() const {
        return messages;
    }

private:
    std::vector<std::string> messages;
};

#define LOG(format, ...) Log(__FILE__, __LINE__, format, ##__VA_ARGS__)

void Log(const char file[], int line, const char* format, ...);

#endif


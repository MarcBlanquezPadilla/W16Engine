#pragma once
#ifndef __LOG_H__
#define __LOG_H__

#include <string>
#include <vector>
#include <functional>

enum LogType {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
};

struct LogInfo
{
    LogType type = LOG_INFO;
    std::string message = "";
    size_t messageHash = 0;
    int count = 0;
};

class LogBuffer
{
public:

    

    static LogBuffer& GetInstance() {
        static LogBuffer instance;
        return instance;
    }

    void AddLog(LogType type, const std::string& msg) {

        size_t incomingHash = std::hash<std::string>{}(msg);

        bool found = false;

        for (auto it = messages.begin(); it != messages.end(); ++it)
        {
            if (it->messageHash == incomingHash && it->type == type)
            {
                if (it->message == msg)
                {
                    LogInfo existingLog = *it;
                    existingLog.count++;

                    messages.erase(it);

                    messages.push_back(existingLog);

                    found = true;
                    break;
                }
            }
        }

        if (!found)
        {
            LogInfo newLog;
            newLog.type = type;
            newLog.message = msg;
            newLog.messageHash = incomingHash;
            newLog.count = 1;

            messages.push_back(newLog);
        }

        if (messages.size() > 1000) {
            messages.erase(messages.begin());
        }
    }

    const std::vector<LogInfo>& GetLogs() const { return messages; }
    void Clear() { messages.clear(); }

private:
    std::vector<LogInfo> messages;
};

#define LOG(type, format, ...) Log(type, __FILE__, __LINE__, format, ##__VA_ARGS__)
void Log(LogType type, const char file[], int line, const char* format, ...);

#endif
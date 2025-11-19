#pragma once

class Event;

class IEventListener
{
public:
    virtual ~IEventListener() = default;

    virtual void OnEvent(const Event& event) = 0;
};
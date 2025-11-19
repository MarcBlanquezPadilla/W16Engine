#pragma once
#include "EventSystem.h"
#include "EventListener.h"
#include "utils/Log.h"

EventSystem::EventSystem(bool startEnabled) : Module(startEnabled)
{
    name = "EventSystem";
    
}

EventSystem::~EventSystem()
{

}

bool EventSystem::Awake()
{
    processingEvents = false;
    return true;
}

bool EventSystem::CleanUp()
{
    ClearQueue();
    return true;
}

void EventSystem::Subscribe(Event::Type eventType, EventListener* listener)
{
	if (!listener) return;

    if (std::find(listeners[eventType].begin(), listeners[eventType].end(), listener) == listeners[eventType].end())
    {
        listeners[eventType].push_back(listener);
        LOG("Listener subscribed to event type %d", static_cast<int>(eventType));
    }
}

void EventSystem::Unsubscribe(Event::Type eventType, EventListener* listener)
{
    if (!listener) return;

    listeners[eventType].erase(
        std::remove(listeners[eventType].begin(), listeners[eventType].end(), listener),
        listeners[eventType].end()
    );
}

void EventSystem::UnsubscribeAll(EventListener* listener)
{
    if (!listener) return;

    for (auto& pair : listeners)
    {
        auto& listenerList = pair.second;
        listenerList.erase(
            std::remove(listenerList.begin(), listenerList.end(), listener),
            listenerList.end()
        );
    }
}

void EventSystem::PublishImmediate(const Event& event)
{
    std::vector<EventListener*> listenersCopy = listeners[event.type];

    for (EventListener* listener : listenersCopy)
    {
        if (listener)
        {
            listener->OnEvent(event);
        }
    }
}

void EventSystem::Publish(std::shared_ptr<Event> event)
{
    if (!event) return;

    eventQueue.push(event);
}

void EventSystem::ProcessEvents()
{
    if (processingEvents)
    {
        LOG("WARNING: Already processing events, skipping nested call");
        return;
    }

    processingEvents = true;

    while (!eventQueue.empty())
    {
        std::shared_ptr<Event> event = eventQueue.front();
        eventQueue.pop();

        if (event)
        {
            PublishImmediate(*event);
        }
    }

    processingEvents = false;
}

void EventSystem::ClearQueue()
{
    while (!eventQueue.empty())
    {
        eventQueue.pop();
    }
}

int EventSystem::GetListenerCount(Event::Type eventType) const
{
    auto it = listeners.find(eventType);
    if (it != listeners.end())
    {
        return it->second.size();
    }
    return 0;
}
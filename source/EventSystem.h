#pragma once
#include "Module.h"
#include <map>
#include "Event.h"

#include <memory>
#include <vector>
#include <queue>


class IEventListener;

class EventSystem : public Module
{
public:
    EventSystem(bool startEnabled);
    virtual ~EventSystem();
    
    bool Awake();

    bool CleanUp();

    void Subscribe(Event::Type eventType, IEventListener* listener);
    void Unsubscribe(Event::Type eventType, IEventListener* listener);
    void UnsubscribeAll(IEventListener* listener);

    void PublishImmediate(const Event& event);
    void Publish(std::shared_ptr<Event> event);

    void ProcessEvents();

    void ClearQueue();

    int GetListenerCount(Event::Type eventType) const;
    int GetQueuedEventCount() const { return eventQueue.size(); }

public:
    std::map<Event::Type, std::vector<IEventListener*>> listeners;
    std::queue<std::shared_ptr<Event>> eventQueue;

    bool processingEvents;


};
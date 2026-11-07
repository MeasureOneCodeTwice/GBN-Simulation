#pragma once

#include "ns3/core-module.h"

#include <queue>
using namespace ns3;

//A queue with a timer that activates a callback when item
//when it expires.
//Items in the queue all have the same lifetime, so the queue expires 
//front to back
//The queue is not modified on item expiry.
template<typename T>
class TimedQueue
{
    typedef std::pair<T, Time> TimedItem;
    public:
        TimedQueue(std::function<void(T)> onExpiryCallback, std::string itemLifetimeDuration);
        void enqueue(const T&);
        T dequeue();
        void clear();
    private:
        void onExpiry();
        void cancelTimeout();
        void scheduleTimeout(Time);
        void assertValidState();

        std::optional<EventId> timeout;
        std::queue<TimedItem> queue;
        Time itemLifetimeDuration;
        std::function<void(T)> onExpiryCallback;
};

#include "./timed-queue.ipp"

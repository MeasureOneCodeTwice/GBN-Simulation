#pragma once

#include "ns3/core-module.h"

#include <queue>
using namespace ns3;

//A queue with a timer that activates a callback and removes an item
//when it expires.
//If multiple items expire at the same time, a callback is called on the head
//of the queue. No items are removed on expiry.
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

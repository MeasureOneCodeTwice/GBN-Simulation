#pragma once

#include "ns3/core-module.h"

#include <queue>
using namespace ns3;

//A queue with a timer that activates a callback and removes an item
//when it expires.
//If multiple items expire at the same time, all are removed and only the first one
//is passed to the callback.
template<typename T>
class TimedQueue
{
    typedef std::pair<T, Time> TimedItem;
    public:
        TimedQueue(std::function<void(T)> onExpiryCallback, std::string itemLifetimeDuration);
        void enqueue(const T&);
        T dequeue();
    private:
        void onExpiry();
        void scheduleTimeout(Time);

        EventId timeout;
        std::queue<TimedItem> queue;
        Time itemLifetimeDuration;
        std::function<void(T)> onExpiryCallback;
};

#include "./timed-queue.ipp"

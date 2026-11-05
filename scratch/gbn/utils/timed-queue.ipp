#pragma once

#include "./timed-queue.h"
#include "assert.h"

template<typename T>
TimedQueue<T>::TimedQueue(std::function<void(T)> callback, std::string itemLifetimeDuration)
    : timeout(),
    queue(),
    itemLifetimeDuration(itemLifetimeDuration),
    onExpiryCallback(callback)
{
}


//what happens when multiple expire at the same time?
template<typename T>
void TimedQueue<T>::enqueue(const T &item) {
    Time expiry = Simulator::Now() + itemLifetimeDuration;

    if(queue.empty()) {
        scheduleTimeout(expiry);
    }

    queue.push(std::pair(item, expiry));
};

template<typename T>
void TimedQueue<T>::scheduleTimeout(Time time) {
    assert(timeout.IsExpired());
    timeout = Simulator::Schedule(time, &TimedQueue<T>::onExpiry, this);
}

template<typename T>
T TimedQueue<T>::dequeue() {
    T &item = queue.front();
    queue.pop();

    if(!queue.empty) {
        Time headExpiry = std::get<1>(queue.front());
        scheduleTimeout(headExpiry);
    } else {
        timeout.Cancel();
    }

    return item;
}

template<typename T>
void TimedQueue<T>::onExpiry() {
    T item = queue.front();

    //remove all items with the same expiry
    T& curr = item;
    while(std::get<1>(curr) == std::get<1>(item)) {
        queue.pop();
    }

    onExpiryCallback(item);
}
#include "timed-queue.ipp"

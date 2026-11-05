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

template<typename T>
void TimedQueue<T>::clear() {
    assertValidState();

    if(!queue.empty()) {
        cancelTimeout();
    }

    queue = std::queue<TimedItem>();
    NS_ASSERT(queue.empty());

    assertValidState();
}

template<typename T>
void TimedQueue<T>::enqueue(const T &item) {
    assertValidState();

    Time expiry = Simulator::Now() + Time(itemLifetimeDuration);
    std::cerr << item.to_string() << " expiry is for " << expiry.As(Time::S) << std::endl;;

    if(queue.empty()) {
        scheduleTimeout(expiry);
    }

    queue.emplace(item, expiry);
    assertValidState();
};


template<typename T>
T TimedQueue<T>::dequeue() {
    assertValidState();
    if(queue.size() == 0) {
        throw std::runtime_error("Cannot dequeue empty queue");
    }

    TimedItem item = queue.front();
    queue.pop();

    if(!queue.empty()) {
        Time headExpiry = std::get<Time>(queue.front());
        scheduleTimeout(headExpiry);
    } else {
        cancelTimeout();
    }

    assertValidState();
    return std::get<T>(item);
}

template<typename T>
void TimedQueue<T>::scheduleTimeout(Time time) {
    if(timeout) {
        cancelTimeout();
    }
    timeout = Simulator::Schedule(time - Simulator::Now(), &TimedQueue<T>::onExpiry, this);
}

template<typename T>
void TimedQueue<T>::cancelTimeout() {
    NS_ASSERT(timeout);
    timeout->Cancel();
    timeout.reset();
}

template<typename T>
void TimedQueue<T>::onExpiry() {
    assertValidState();
    NS_ASSERT(!queue.empty());
    TimedItem item = queue.front();

    onExpiryCallback(std::get<T>(item));
    assertValidState();
}

template<typename T>
void TimedQueue<T>::assertValidState() {
    NS_ASSERT(timeout.has_value() == !queue.empty());
}

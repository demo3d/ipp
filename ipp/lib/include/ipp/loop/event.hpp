#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include "message.hpp"

namespace ipp {
namespace loop {

/**
 * @brief Event is a Message instance that is created by a source System to signal something.
 * Events are dispatched to all other systems and to Event listeners.
 *
 * Unlike Message and Command messages Events can (and should) be dispatched immediately to all
 * listeners without going to message queue, this allows Systems to signal other systems during
 * current frame.
 */
class Event : public Message {
private:
    SystemBase& _source;

public:
    Event(uint32_t messageTypeId, SystemBase& source)
        : Message(messageTypeId)
        , _source{source}
    {
    }

    /**
     * @brief Untyped pointer to event data
     */
    virtual const void* getDataPtr() const = 0;

    /**
     * @brief Event data pointer size in bytes
     */
    virtual size_t getDataSize() const = 0;

    /**
     * @brief Returns @see Kind::Event
     */
    Kind getMessageKind() const override
    {
        return Kind::Event;
    }

    /**
     * @brief System that created the Event object
     */
    SystemBase& getSource() const
    {
        return _source;
    }
};

/**
 * @brief Generic implementation of Event that stores T as const data
 *
 * T must be copyable or movable.
 */
template <typename T>
class EventT final : public Event {
public:
    typedef T DataType;

private:
    T _data;

public:
    EventT(SystemBase& sender, const T& data)
        : Event(GetTypeId(), sender)
        , _data{data}
    {
    }

    EventT(SystemBase& sender, T&& data)
        : Event(GetTypeId(), sender)
        , _data{std::move(data)}
    {
    }

    /**
     * @brief Return data const reference
     */
    const T& getData() const
    {
        return _data;
    }

    /**
     * @brief Data pointer
     */
    const void* getDataPtr() const override
    {
        return &_data;
    }

    /**
     * @brief
     */
    size_t getDataSize() const override
    {
        return sizeof(T);
    }

    /**
     * @brief Returns @see EventT<T>::EventTypeName
     */
    const std::string& getMessageTypeName() const override
    {
        return EventT<T>::EventTypeName;
    }

    /**
     * @brief EventT<T> specific globally unique Message name
     */
    static const std::string EventTypeName;

    /**
     * @brief EventT<T> specific globally unique Message type id number (>0)
     */
    static uint32_t GetTypeId()
    {
        static uint32_t typeId = Message::MessageTypeIdCounter++;
        return typeId;
    }
};
}
}

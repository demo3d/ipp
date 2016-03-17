#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include "message.hpp"

namespace ipp {
namespace loop {

/**
 * @brief Command is a Message dispatched to a specific System to perform an action
 *
 * Commands can only be sent to a specific System and are dispatched before Events and Messages.
 */
class Command : public Message {
public:
    /**
     * @brief Interface for creating a Command message instance from untyped data pointer
     */
    class Factory : public NonCopyable {
    private:
        SystemBase& _receiver;

    public:
        Factory(SystemBase& receiver)
            : _receiver{receiver}
        {
        }
        virtual ~Factory() = default;

        /**
         * @brief Receiver System for Factory instances of Command implementation type
         */
        SystemBase& getReceiver() const
        {
            return _receiver;
        }

        /**
         * @brief Created Command implementation type globally unique name string
         */
        virtual const std::string& getMessageTypeName() const = 0;

        /**
         * @brief Created Command implementation type globally unique id number
         */
        virtual uint32_t getMessageTypeId() const = 0;

        /**
         * @brief Create a new instance of Command from data buffer
         * @note data is only valid during the create call, implementation must not keep references
         */
        virtual std::unique_ptr<Command> create(const void* data) const = 0;
    };

private:
    SystemBase& _receiver;

public:
    Command(uint32_t messageTypeId, SystemBase& receiver)
        : Message(messageTypeId)
        , _receiver{receiver}
    {
    }

    /**
     * @brief System that to which this Command will be dispatched to
     */
    SystemBase& getReceiver() const
    {
        return _receiver;
    }

    /**
     * @brief Returns @see Kind::Command
     */
    Kind getMessageKind() const override
    {
        return Kind::Command;
    }
};

/**
 * @brief Generic implementation of Command that stores T as const data
 *
 * T must be copyable (can optionally be moveable).
 */
template <typename T>
class CommandT final : public Command {
public:
    typedef T DataType;

public:
    class Factory final : public Command::Factory {
    public:
        Factory(SystemBase& receiver)
            : Command::Factory(receiver)
        {
        }

        const std::string& getMessageTypeName() const override
        {
            return CommandT<T>::CommandTypeName;
        }

        uint32_t getMessageTypeId() const override
        {
            return CommandT<T>::GetTypeId();
        }

        std::unique_ptr<Command> create(const void* data) const override
        {
            return std::make_unique<CommandT<T>>(getReceiver(), data);
        }
    };

private:
    T _data;

public:
    CommandT(SystemBase& receiver, const void* data)
        : Command(GetTypeId(), receiver)
        , _data{*reinterpret_cast<const T*>(data)}
    {
    }

    CommandT(SystemBase& receiver, const T& data)
        : Command(GetTypeId(), receiver)
        , _data{data}
    {
    }

    /**
     * @brief Const message data access
     */
    const T& getData() const
    {
        return _data;
    }

    /**
     * @brief Returns @see CommandT<T>::CommandTypeName
     */
    const std::string& getMessageTypeName() const override
    {
        return CommandT<T>::CommandTypeName;
    }

    /**
     * @brief CommandT<T> specific globally unique Message name
     */
    static const std::string CommandTypeName;

    /**
     * @brief CommandT<T> specific globally unique Message type id number (>0)
     */
    static uint32_t GetTypeId()
    {
        static uint32_t typeId = Message::MessageTypeIdCounter++;
        return typeId;
    }
};
}
}

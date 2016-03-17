#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>

namespace ipp {
namespace loop {

/**
 * @brief Abstract interface for Message objects dispatched trough MessageLoop
 */
class Message : public NonCopyable {
public:
    /**
     * @brief Message Kind states which interface and semantics a Message type implements
     */
    enum class Kind {
        /**
         * @brief Type implements only the most generic Message interface
         * These messages are dispatched after Commands and Events
         */
        Message = 0,

        /**
         * @brief Type implements Event inteface, see Event class documentation for more info
         */
        Event,

        /**
         * @brief Type implements Command interface, see Command class documentation for more info
         */
        Command
    };

private:
    uint32_t _messageTypeId;

public:
    Message(uint32_t messageTypeId)
        : _messageTypeId{messageTypeId}
    {
    }

    virtual ~Message() = default;

    /**
     * @brief Message implementation type globally unique id number (> 1)
     */
    uint32_t getMessageTypeId() const
    {
        return _messageTypeId;
    }

    /**
     * @brief Message implementation type globally unique name string
     */
    virtual const std::string& getMessageTypeName() const = 0;

    /**
     * @brief Message implementation @see Kind, @see Kind::Message by default.
     */
    virtual Kind getMessageKind() const
    {
        return Kind::Message;
    }

    /**
     * @brief Shared global counter holding the next free message id, starting at 1
     * Use in every concrete Message type implementation to get a unique message id and then
     * increment the counter (++)
     */
    static uint32_t MessageTypeIdCounter;
};
}
}

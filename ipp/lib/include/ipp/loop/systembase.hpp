#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include <ipp/log.hpp>
#include "message.hpp"

namespace ipp {
namespace loop {

/**
 * @brief MessageLoop interface for systems.
 *
 * Use SystemT as base class to implement a new System.
 */
class SystemBase : public NonCopyable {
public:
    template <typename T>
    friend class SystemT;
    friend class MessageLoop;

private:
    static uint32_t SystemTypeIdCounter;
    MessageLoop& _messageLoop;

private:
    /**
     * @brief Called by MessageLoop initialize method to allow System to initialize state
     * @return Update dependancies of SystemBase, must be members of same MessageLoop and can't
     *         also depend on this System (no cycles allowed).
     */
    virtual std::vector<SystemBase*> initialize()
    {
        return {};
    }

    /**
     * @brief Called by MessageLoop to push new messages
     */
    virtual void onMessage(const Message& message)
    {
    }

    /**
     * @brief Called by MessageLoop::update method to dispatch updates.
     */
    virtual void onUpdate()
    {
    }

public:
    SystemBase(MessageLoop& messageLoop)
        : _messageLoop{messageLoop}
    {
    }

    virtual ~SystemBase() = default;

    /**
     * @brief System implementation type unique name string.
     */
    virtual const std::string& getSystemTypeName() const = 0;

    /**
     * @brief Unique System id for implementation type.
     */
    virtual uint32_t getSystemTypeId() const = 0;

    /**
     * @brief Owning MessageLoop instance.
     */
    MessageLoop& getMessageLoop() const
    {
        return _messageLoop;
    }
};
}
}

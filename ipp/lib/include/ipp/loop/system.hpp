#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include <ipp/log.hpp>
#include "messageloop.hpp"

namespace ipp {
namespace loop {

/**
 * @brief Base class for MessageLoop Systems, T must be System implementation class.
 *
 * Provides common SystemBase implementation utility methods.
 *
 * @note Every derived type must implement static SystemTypeName for T.
 */
template <typename T>
class SystemT : public SystemBase {
protected:
    /**
     * @brief Create EventT<T> and dispatch it trough parent @see MessageLoop
     * Event data is constructed using params and this System is Event source
     */
    template <typename E, typename... Params>
    void dispatchEventT(Params&&... params)
    {
        getMessageLoop().dispatchEvent(
            std::make_unique<E>(*this, typename E::DataType(std::forward<Params>(params)...)));
    }

    /**
     * @brief Create C::Factory instance with this as receiver and register it to @see MessageLoop
     * @note C must be CommandT<> template instance.
     */
    template <typename C>
    void registerCommandT()
    {
        getMessageLoop().registerCommandFactory(std::make_unique<typename C::Factory>(*this));
    }

    /**
     * @brief Register EventT type with EventTypeName and GetTypeId to @see MessageLoop
     * @note E must be EventT<> template instance.
     */
    template <typename E>
    void registerEventT()
    {
        getMessageLoop().registerMessageType(E::EventTypeName, E::GetTypeId());
    }

    /**
     * @brief System implementation utility for accessing EventT data
     * @return Pointer to E::DataType if message is EventT<T> specified by E or nullptr
     */
    template <typename E>
    static const typename E::DataType* getEventData(const Message& message)
    {
        if (message.getMessageTypeId() != E::GetTypeId()) {
            return nullptr;
        }

        auto event = dynamic_cast<const E*>(&message);
        if (!event) {
            return nullptr;
        }
        return &event->getData();
    }

    /**
     * @brief System implementation utility for accessing CommandT data
     * @return Pointer to C::DataType if message is CommandT<T> specified by C or nullptr
     */
    template <typename C>
    static const typename C::DataType* getCommandData(const Message& message)
    {
        if (message.getMessageTypeId() != C::GetTypeId()) {
            return nullptr;
        }

        auto command = dynamic_cast<const C*>(&message);
        if (!command) {
            return nullptr;
        }
        return &command->getData();
    }

public:
    SystemT(MessageLoop& messageLoop)
        : SystemBase(messageLoop)
    {
    }

    /**
     * @brief Override getComponentTypeId to return T::GetComponentTypeId
     */
    uint32_t getSystemTypeId() const override
    {
        return T::GetSystemTypeId();
    }

    /**
     * @brief Return System name string from SystemT<T>::SystemTypeName.
     */
    const std::string& getSystemTypeName() const override
    {
        return SystemT<T>::SystemTypeName;
    }

    /**
     * @brief Returns a unique integer for every type T.
     */
    static uint32_t GetSystemTypeId()
    {
        static uint32_t systemTypeId = SystemBase::SystemTypeIdCounter++;
        return systemTypeId;
    }

    /**
     * @brief Unique system implementation type name.
     */
    static const std::string SystemTypeName;
};
}
}

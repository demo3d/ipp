#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include "message.hpp"
#include "event.hpp"
#include "command.hpp"
#include "systembase.hpp"

namespace ipp {
namespace loop {

/**
 * @brief A collection of systems that updates and dispatches messages to all of them.
 */
class MessageLoop : public ipp::NonCopyable {
public:
    /**
     * @brief Listener object that is used by MessageLoop to dispatch requested Message types
     */
    class EventListener : public NonCopyable {
    public:
        friend class MessageLoop;

    private:
        MessageLoop& _messageLoop;
        std::function<void(uint32_t, const Event*)> _callback;

        EventListener(MessageLoop& messageLoop,
                      std::function<void(uint32_t, const Event*)> callback)
            : _messageLoop{messageLoop}
            , _callback{callback}
        {
        }

    public:
        /**
         * @brief Message loop this listener belongs to.
         */
        MessageLoop& getMessageLoop() const
        {
            return _messageLoop;
        }
    };

private:
    bool _initialized;
    std::vector<std::unique_ptr<SystemBase>> _systems;
    std::unique_ptr<std::vector<std::unique_ptr<Message>>> _messageQueueActive;
    std::unique_ptr<std::vector<std::unique_ptr<Message>>> _messageQueueProcessing;
    std::vector<std::unique_ptr<EventListener>> _eventListeners;
    std::vector<std::unique_ptr<Command::Factory>> _commandFactories;
    std::vector<std::string> _messageTypeNames;

public:
    MessageLoop()
        : _initialized{false}
    {
    }

    /**
     * @brief Call initialize on all Systems to signal that all Systems have been created.
     *
     * Can only be called once.
     * Entities can only be created after this method has completed.
     */
    void initialize();

    /**
     * @brief Has initialize function been called and complete successfully.
     */
    bool isInitialized() const
    {
        return _initialized;
    }

    /**
     * @brief Create a new System T for MessageLoop.
     * @note Systems must be created before initialize method is called.
     */
    template <typename T, typename... Params>
    T& createSystem(Params&&... params)
    {
        if (_initialized) {
            IVL_LOG_THROW_ERROR(std::runtime_error,
                                "MessageLoop already initialized, cannot create a new System.");
        }

        auto systemTypeId = T::GetSystemTypeId();
        auto it = _systems.begin();
        while (it != _systems.end()) {
            if (systemTypeId == (*it)->getSystemTypeId() ||
                T::SystemTypeName == (*it)->getSystemTypeName()) {
                IVL_LOG_THROW_ERROR(std::runtime_error,
                                    "MessageLoop already contains requested System type {} ({})",
                                    T::SystemTypeName, systemTypeId);
            }
            if (systemTypeId < (*it)->getSystemTypeId()) {
                break;
            }
            ++it;
        }

        auto system = std::make_unique<T>(*this, std::forward<Params>(params)...);
        auto result = system.get();
        _systems.emplace(it, std::move(system));

        return *result;
    }

    /**
     * @brief Get existing System of type T.
     * @return nullptr if no such System exists in loop, System T pointer otherwise.
     */
    template <typename T>
    T* findSystem() const
    {
        return dynamic_cast<T*>(this->findSystem(T::GetSystemTypeId()));
    }

    /**
     * @brief Get existing SystemBase
     */
    SystemBase* findSystem(uint32_t systemTypeId) const;

    /**
     * @brief Get existing SystemBase implementation with specified name.
     */
    SystemBase* findSystem(const std::string& name) const;

    /**
     * @brief Systems associated with MessageLoop.
     */
    const std::vector<std::unique_ptr<SystemBase>>& getSystems() const
    {
        return _systems;
    }

    /**
     * @brief Register @see Command::Factory with unique message type
     */
    void registerCommandFactory(std::unique_ptr<Command::Factory> factory);

    /**
     * @brief Register @see Message implementation with specified typeName and typeId
     */
    void registerMessageType(const std::string& typeName, uint32_t typeId);

    /**
     * @brief Return a uinique type id associated with message type
     */
    uint32_t findMessageTypeId(const std::string& typeName) const;

    /**
     * @brief Return a unique type name associated with message type id
     */
    const std::string& findMessageTypeName(uint32_t typeId) const;

    /**
     * @brief Get registered @see Command::Factory with specified type id
     * @note Performs indexed lookup (very efficient)
     */
    Command::Factory* findCommandFactory(uint32_t typeId) const;

    /**
     * @brief Get registered @see Command::Factory with specified type name
     * @note Performs a linear comparison search (not efficient)
     */
    Command::Factory* findCommandFactory(const std::string& typeName) const;

    /**
     * @brief Add a message listener callback to MessageLoop.
     */
    EventListener* createListener(std::function<void(uint32_t, const Event*)> callback);

    /**
     * @brief Remove a listener from message loop.
     * After this call listener instance will be invalid (deleted).
     */
    void releaseListener(EventListener* listener);

    /**
     * @brief Immediately dispatch a Event Message.
     *
     * Events can be dispatched without going trough Message queue to allow other systems to
     * respond to changes immediately.
     *
     * @note This call is only valid on MessageLoop thread.
     */
    void dispatchEvent(std::unique_ptr<Event> event);

    /**
     * @brief Place message in to a queue that will be dispatched on next update
     */
    void enqueueMessage(std::unique_ptr<Message> message);

    /**
     * @brief Create a new Command message from type and data and add it to message queue
     */
    void enqueueCommand(uint32_t typeId, const void* data);

    /**
     * @brief Create a new SystemBase::CommandT<T> instance from data copy
     */
    template <typename C, typename... Params>
    void enqueueCommandT(Params&&... params)
    {
        auto factory = findCommandFactory(C::GetTypeId());
        if (factory == nullptr) {
            IVL_LOG_THROW_ERROR(std::logic_error, "Command Type {} not registered with MessageLoop",
                                C::CommandTypeName);
        }
        enqueueMessage(std::make_unique<C>(factory->getReceiver(),
                                           typename C::DataType(std::forward<Params>(params)...)));
    }

    /**
     * @brief Update message loop by dispatching all queued messages and updating every system.
     */
    void update();
};
}
}

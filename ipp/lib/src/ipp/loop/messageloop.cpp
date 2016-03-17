#include <ipp/loop/messageloop.hpp>
#include <sstream>

using namespace std;
using namespace ipp::loop;

uint32_t SystemBase::SystemTypeIdCounter = 1;

void MessageLoop::initialize()
{
    if (_initialized) {
        IVL_LOG_THROW_ERROR(runtime_error,
                            "MessageLoop is already initialized, cannot initialize twice.");
    }

    // allocate message queues
    _messageQueueActive = make_unique<vector<unique_ptr<Message>>>();
    _messageQueueProcessing = make_unique<vector<unique_ptr<Message>>>();

    // initialize all systems and strore dependencies
    unordered_map<SystemBase*, vector<SystemBase*>> dependencyMap;
    for (auto& system : _systems) {
        dependencyMap.emplace(system.get(), system->initialize());
    }

    // move systems out of _systems for rebuilding and transfer ownership to a collection of
    // systems whose dependancies have not been resolved
    unordered_map<SystemBase*, unique_ptr<SystemBase>> unresolvedSystems;
    for (auto& system : _systems) {
        unresolvedSystems.emplace(system.get(), move(system));
    }
    _systems.clear();

    // systems that got their dependancies resolved in current iteration
    std::vector<unique_ptr<SystemBase>> newResolvedSystems;

    // first iteration of looking for systems finds systems with no dependencies
    for (auto& system : unresolvedSystems) {
        if (dependencyMap[system.first].size() == 0) {
            newResolvedSystems.push_back(move(system.second));
        }
    }

    // loop while we keep adding new systems to initialized list
    while (newResolvedSystems.size() > 0) {
        // move satisfied systems from previous iteration to _systems in correct order
        for (auto& system : newResolvedSystems) {
            auto ptr = system.get();
            _systems.push_back(move(system));
            unresolvedSystems.erase(ptr);
        }
        newResolvedSystems.clear();

        // loop trough uninitialized systems and check if all dependencies have now been satisfied
        for (auto& system : unresolvedSystems) {
            auto& dependencies = dependencyMap[system.first];
            size_t dependenciesFound = 0;
            // count number of satisfied dependencies
            for (auto& resolved : _systems) {
                if (find(dependencies.begin(), dependencies.end(), resolved.get()) !=
                    dependencies.end()) {
                    dependenciesFound++;
                }
            }
            // if all dependencies are satisfied move system to satisfied list
            if (dependencies.size() == dependenciesFound) {
                newResolvedSystems.push_back(move(system.second));
            }
        }
    }

    // if some system dependencies could not be resolved raise an error
    if (unresolvedSystems.size() > 0) {
        stringstream unresolvedList;
        for (auto& system : unresolvedSystems) {
            unresolvedList << system.first->getSystemTypeName() << " (";
            bool first = true;
            for (auto dependency : dependencyMap[system.first]) {
                if (first) {
                    first = false;
                }
                else {
                    unresolvedList << ", ";
                }
                unresolvedList << dependency->getSystemTypeName();
            }
            unresolvedList << ")" << endl;
        }

        IVL_LOG_THROW_ERROR(runtime_error,
                            "MessageLoop contains Systems with unresolved dependencies : \n{}",
                            unresolvedList.str());
    }

    _initialized = true;
}

SystemBase* MessageLoop::findSystem(const string& name) const
{
    for (auto& system : _systems) {
        if (system->getSystemTypeName() == name) {
            return system.get();
        }
    }
    return nullptr;
}

SystemBase* MessageLoop::findSystem(uint32_t systemTypeId) const
{
    for (auto& system : _systems) {
        if (system->getSystemTypeId() == systemTypeId) {
            return system.get();
        }
    }
    return nullptr;
}

MessageLoop::EventListener* MessageLoop::createListener(
    function<void(uint32_t, const Event*)> callback)
{
    auto listener = new EventListener(*this, callback);
    _eventListeners.emplace_back(listener);
    return listener;
}

void MessageLoop::releaseListener(EventListener* listener)
{
    auto listenerIt = find_if(_eventListeners.begin(), _eventListeners.end(),
                              [listener](const auto& elem) { return elem.get() == listener; });
    if (listenerIt != _eventListeners.end()) {
        _eventListeners.erase(listenerIt);
    }
}

void MessageLoop::registerCommandFactory(std::unique_ptr<Command::Factory> factory)
{
    auto typeIndex = factory->getMessageTypeId() - 1;
    if (typeIndex >= _commandFactories.size()) {
        _commandFactories.resize(typeIndex + 1);
    }
    if (typeIndex >= _messageTypeNames.size()) {
        _messageTypeNames.resize(typeIndex + 1);
    }
    _messageTypeNames[typeIndex] = factory->getMessageTypeName();
    _commandFactories[typeIndex] = move(factory);
}

void MessageLoop::registerMessageType(const string& typeName, uint32_t typeId)
{
    auto typeIndex = typeId - 1;
    if (typeIndex >= _messageTypeNames.size()) {
        _messageTypeNames.resize(typeIndex + 1);
    }
    IVL_LOG(Trace, "Registering message type {} with type Id {}", typeName, typeId);
    _messageTypeNames[typeIndex] = typeName;
}

uint32_t MessageLoop::findMessageTypeId(const std::string& typeName) const
{
    auto typeIt = find_if(_messageTypeNames.begin(), _messageTypeNames.end(),
                          [&typeName](const auto& t) { return t == typeName; });
    if (typeIt == _messageTypeNames.end()) {
        return 0;
    }
    return static_cast<uint32_t>(distance(_messageTypeNames.begin(), typeIt) + 1);
}

const std::string& MessageLoop::findMessageTypeName(uint32_t typeId) const
{
    static const std::string empty;
    auto typeIndex = typeId - 1;
    if (typeIndex >= _messageTypeNames.size()) {
        return empty;
    }
    return _messageTypeNames[typeIndex];
}

Command::Factory* MessageLoop::findCommandFactory(uint32_t typeId) const
{
    auto typeIndex = typeId - 1;
    if (typeIndex >= _commandFactories.size()) {
        return nullptr;
    }
    return _commandFactories[typeIndex].get();
}

Command::Factory* MessageLoop::findCommandFactory(const std::string& typeName) const
{
    auto typeIt = find_if(
        _commandFactories.begin(), _commandFactories.end(),
        [&typeName](const auto& factory) { return factory->getMessageTypeName() == typeName; });
    if (typeIt == _commandFactories.end()) {
        return nullptr;
    }
    return typeIt->get();
}

void MessageLoop::dispatchEvent(unique_ptr<Event> event)
{
    for (auto& system : _systems) {
        if (&event->getSource() != system.get()) {
            system->onMessage(*event);
        }
    }

    for (const auto& listener : _eventListeners) {
        listener->_callback(event->getMessageTypeId(), event.get());
    }
}

void MessageLoop::enqueueMessage(unique_ptr<Message> message)
{
    _messageQueueActive->push_back(move(message));
}

void MessageLoop::enqueueCommand(uint32_t typeId, const void* data)
{
    auto factory = findCommandFactory(typeId);
    if (!factory) {
        IVL_LOG_THROW_ERROR(logic_error, "Unknown command type id {}", typeId);
    }
    enqueueMessage(factory->create(data));
}

void MessageLoop::update()
{
    // ping-pong message queue buffers,
    // active is collecting messages for the next iteration,
    // processing is being dispatched in this iteration
    swap(_messageQueueActive, _messageQueueProcessing);

    // dispatch queued commands before all other messages
    for (auto& message : *_messageQueueProcessing) {
        if (message->getMessageKind() == Message::Kind::Command) {
            auto command = dynamic_cast<Command*>(message.get());
            command->getReceiver().onMessage(*command);
        }
    }

    // dispatch events before custom messages
    for (auto& message : *_messageQueueProcessing) {
        if (message->getMessageKind() == Message::Kind::Event) {
            auto event = dynamic_cast<Event*>(message.get());
            for (auto& system : _systems) {
                if (&event->getSource() != system.get()) {
                    system->onMessage(*event);
                }
            }

            for (const auto& listener : _eventListeners) {
                listener->_callback(event->getMessageTypeId(), event);
            }
        }
    }

    // dispatch all other queued messages
    for (auto& message : *_messageQueueProcessing) {
        if (message->getMessageKind() == Message::Kind::Message) {
            for (auto& system : _systems) {
                system->onMessage(*message);
            }
        }
    }

    // clear queue when finished dispatching all messages
    _messageQueueProcessing->clear();

    // update all systems
    for (auto& system : _systems) {
        system->onUpdate();
    }
}

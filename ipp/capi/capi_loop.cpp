#include <ipp/loop/messageloop.hpp>
#include "capi.hpp"

using namespace std;
using namespace ipp;
using namespace ipp::loop;

extern "C" {
typedef void (*EventListenerCallback)(uint32_t typeId, uint32_t dataSize, const void* eventData);

/**
 * @brief Register a message listener callback function to loop and return listener handle
 */
MessageLoop::EventListener* IVL_API_EXPORT
loop_create_event_listener(MessageLoop* loop, EventListenerCallback callback)
{
    return loop->createListener([callback](uint32_t typeId, const Event* event) {
        callback(typeId, static_cast<uint32_t>(event->getDataSize()), event->getDataPtr());
    });
}

/**
 * @brief Unregister a message listener (referenced by listener handle) from loop
 */
void IVL_API_EXPORT loop_release_listener(MessageLoop* loop, MessageLoop::EventListener* listener)
{
    loop->releaseListener(listener);
}

/**
 * @brief Return unique Message type name from unique type id or empty string if no type found
 */
const char* IVL_API_EXPORT loop_find_message_type_name(MessageLoop* loop, uint32_t typeId)
{
    return loop->findMessageTypeName(typeId).c_str();
}

/**
 * @brief Return unique Message type Id from unique type name string or 0 if no type found
 */
uint32_t IVL_API_EXPORT loop_find_message_type_id(MessageLoop* loop, const char* name)
{
    return loop->findMessageTypeId(name);
}

/**
 * @brief Return c_str message type name
 */
const char* IVL_API_EXPORT loop_message_get_type_name(Message* message)
{
    return message->getMessageTypeName().c_str();
}

/**
 * @brief Return int message type id
 */
uint32_t IVL_API_EXPORT loop_message_get_type_id(Message* message)
{
    return message->getMessageTypeId();
}

/**
 * @brief Deserialize a Command of typeId from data and enqueue it in MessageLoop
 * @note data reference is not held after the function returns
 */
void IVL_API_EXPORT loop_enqueue_command(MessageLoop* loop, uint32_t typeId, const void* data)
{
    loop->enqueueCommand(typeId, data);
}

/**
 * @brief Update loop by performing a loop iteration
 */
void IVL_API_EXPORT loop_update(MessageLoop* loop)
{
    loop->update();
}
}

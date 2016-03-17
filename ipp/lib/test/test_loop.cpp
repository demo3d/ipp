#include <catch.hpp>
#include <flatbuffers/flatbuffers.h>
#include <ipp/loop/messageloop.hpp>

using namespace std;
using namespace ipp::loop;

template <int N>
class DummyMessage final : public Message {
public:
    DummyMessage(int message)
        : Message(GetTypeId())
        , message{message}
    {
    }

    const std::string& getMessageTypeName() const override
    {
        return MessageTypeName;
    }

    int message;

    /**
     * @brief DummyMessage<N> specific globally unique Message type id number (>0)
     */
    static uint32_t GetTypeId()
    {
        static uint32_t typeId = Message::MessageTypeIdCounter++;
        return typeId;
    }

    static const std::string MessageTypeName;
};

template <int N>
class DummySystem : public SystemT<DummySystem<N>> {
private:
public:
    DummySystem(MessageLoop& messageLoop)
        : SystemT<DummySystem<N>>(messageLoop)
    {
    }

    std::vector<int> messages;

    static const string SystemTypeName;
};

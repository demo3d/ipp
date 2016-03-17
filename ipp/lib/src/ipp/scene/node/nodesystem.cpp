#include <ipp/scene/node/nodesystem.hpp>

using namespace std;
using namespace glm;
using namespace ipp::loop;
using namespace ipp::scene::node;

template <>
const std::string SystemT<NodeSystem>::SystemTypeName = "NodeSystem";

void NodeSystem::onUpdate()
{
    // update node transforms
    _rootNode.updateTransform();
}

NodeSystem::NodeSystem(MessageLoop& messageLoop)
    : SystemT<NodeSystem>(messageLoop)
{
}

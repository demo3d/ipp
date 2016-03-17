#pragma once

#include <ipp/shared.hpp>
#include <ipp/loop/system.hpp>
#include "node.hpp"
#include "nodecomponent.hpp"

namespace ipp {
namespace scene {
namespace node {

/**
 * @brief Scene Loop system that updates all Node components attached to root node.
 */
class NodeSystem final : public loop::SystemT<NodeSystem> {
private:
    Node _rootNode;

    /**
     * @brief Update Node tree matrices to reflect transform changes.
     */
    void onUpdate() override;

public:
    NodeSystem(loop::MessageLoop& messageLoop);

    /**
     * @brief Root scene node.
     * Scene nodes must be a descendant of this node to get updated correctly.
     */
    Node& getRootNode()
    {
        return _rootNode;
    }

    /**
     * @brief Root scene node.
     * Scene nodes must be a descendant of this node to get updated correctly.
     */
    const Node& getRootNode() const
    {
        return _rootNode;
    }
};
}
}
}

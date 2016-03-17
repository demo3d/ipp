#pragma once

#include <ipp/shared.hpp>
#include <ipp/entity/component.hpp>
#include "node.hpp"

namespace ipp {
namespace scene {
namespace node {

/**
 * @brief Implements ComponentT and Node class to create a NodeComponent type.
 */
class NodeComponent : public Node, public entity::ComponentT<NodeComponent> {
public:
    NodeComponent(entity::Entity& entity)
        : entity::ComponentT<NodeComponent>(entity)
    {
    }
};
}
}
}

#pragma once

#include <ipp/shared.hpp>
#include <ipp/log.hpp>
#include "nodetransform.hpp"

namespace ipp {
namespace scene {
namespace node {

/**
 * @brief Generic tree Node with hierarchical 3D transformation.
 */
class Node {
private:
    Node* _parent;
    std::vector<Node*> _children;
    bool _hidden;
    NodeTransform _transform;
    glm::mat4 _transformLocalMatrix;
    glm::mat4 _transformMatrix;
    glm::mat4 _transformParentingInverseMatrix;

public:
    Node();
    virtual ~Node();

    /**
     * @brief Add a node as a child to this node.
     */
    void addChild(Node* child, const glm::mat4& transformParentingInverseMatrix);

    /**
     * @brief Remove a child node from this node.
     */
    void removeChild(Node* child);

    /**
     * @brief Update node transform matrices before drawing.
     *
     * Calls updateTransform() on children recursively.
     */
    void updateTransform();

    /**
     * @brief Is node hidden, true if node set to hidden or any parent is hidden.
     */
    bool isHidden() const;

    /**
     * @brief Set node hidden/visible (also makes all child nodes hidden if true).
     */
    void setHidden(bool hidden)
    {
        _hidden = hidden;
    }

    /**
     * @brief Parent relative transform properties.
     *
     * Changes to returned reference will be reflected in transform matrix after updateTransform
     */
    NodeTransform& getTransform()
    {
        return _transform;
    }

    /**
     * @brief Parent relative transform properties.
     */
    const NodeTransform& getTransform() const
    {
        return _transform;
    }

    /**
     * @brief Parent relative transform matrix.
     * @note Only valid once update finishes after every transform property change.
     */
    const glm::mat4& getTransformMatrix() const
    {
        return _transformMatrix;
    }

    /**
     * @brief Absolute transform matrix (includes parent transform).
     * @note Only valid once update finishes after every transform property change.
     */
    const glm::mat4& getTransformLocalMatrix() const
    {
        return _transformLocalMatrix;
    }

    /**
     * @brief Parent node reference.
     * Reference is valid as long as this node is valid and parent doesn't change.
     * Returned value is nullptr if node is not attached to a parent (eg. root node)
     */
    Node* getParent()
    {
        return _parent;
    }
};
}
}
}

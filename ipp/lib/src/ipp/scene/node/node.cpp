#include <ipp/scene/node/node.hpp>
#include <ipp/scene/node/nodecomponent.hpp>

using namespace std;
using namespace glm;
using namespace ipp::resource;
using namespace ipp::entity;
using namespace ipp::render;
using namespace ipp::scene::node;

template <>
const std::string ComponentT<NodeComponent>::ComponentTypeName = "SceneNodeComponent";

Node::Node()
    : _parent{nullptr}
    , _hidden{false}
{
}

Node::~Node()
{
    for (auto child : _children) {
        child->_parent = nullptr;
    }

    if (_parent != nullptr) {
        _parent->removeChild(this);
    }
}

void Node::updateTransform()
{
    _transformLocalMatrix = static_cast<mat4>(_transform);
    if (_parent != nullptr) {
        _transformMatrix = _parent->getTransformMatrix() * _transformParentingInverseMatrix *
                           _transformLocalMatrix;
    }
    else {
        _transformMatrix = _transformLocalMatrix;
    }

    for (auto& child : _children) {
        child->updateTransform();
    }
}

bool Node::isHidden() const
{
    if (_hidden) {
        return true;
    }
    if (_parent != nullptr) {
        return _parent->isHidden();
    }
    return false;
}

void Node::addChild(Node* child, const mat4& transformParentingInverseMatrix)
{
    child->_parent = this;
    child->_transformParentingInverseMatrix = transformParentingInverseMatrix;
    _children.push_back(child);
}

void Node::removeChild(Node* child)
{
    for (auto childIt = _children.begin(); childIt != _children.end(); ++childIt) {
        if (*childIt == child) {
            _children.erase(childIt);
            child->_parent = nullptr;
            return;
        }
    }
    throw std::range_error("Unable to find requested child node");
}

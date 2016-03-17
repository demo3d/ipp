#include <ipp/entity/world.hpp>

using namespace std;
using namespace ipp::entity;

void Entity::dispatchComponentsModified()
{
    _world.onEntityComponentsModified(*this);
}

ComponentBase* Entity::findComponent(uint32_t componentTypeId) const
{
    auto componentIt = std::find_if(_componentBuffer.begin(), _componentBuffer.end(),
                                    [componentTypeId](auto& component) {
                                        return componentTypeId == component->getComponentTypeId();
                                    });
    if (componentIt == _componentBuffer.end()) {
        return nullptr;
    }
    return componentIt->get();
}

ComponentBase* Entity::findComponent(const std::string& componentTypeName) const
{
    auto componentIt = std::find_if(
        _componentBuffer.begin(), _componentBuffer.end(), [componentTypeName](auto& component) {
            return componentTypeName == component->getComponentTypeName();
        });
    if (componentIt == _componentBuffer.end()) {
        return nullptr;
    }
    return componentIt->get();
}

/**
 * @brief Remove component from Entity.
 */
void Entity::removeComponent(ComponentBase* component)
{
    assert(component != nullptr);
    assert(&component->getEntity() == this);

    auto componentIt = std::find_if(
        _componentBuffer.begin(), _componentBuffer.end(),
        [component](auto& componentInstance) { return componentInstance.get() == component; });

    assert(componentIt != _componentBuffer.end());

    auto componentInstance = std::move(*componentIt);
    _componentBuffer.erase(componentIt);
    dispatchComponentsModified();
}

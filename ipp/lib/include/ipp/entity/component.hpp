#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include <ipp/log.hpp>
#include <ipp/loop/message.hpp>

namespace ipp {
namespace entity {

/**
 * @brief Entity interface for Component access, derive from ComponentT to create a Component type.
 */
class ComponentBase : public NonCopyable {
public:
    template <typename T>
    friend class ComponentT;

private:
    static uint32_t ComponentTypeIdCounter;
    Entity& _entity;

public:
    ComponentBase(Entity& entity)
        : _entity{entity}
    {
    }

    virtual ~ComponentBase() = default;

    /**
     * @brief Unique Component implementation type name string.
     */
    virtual const std::string& getComponentTypeName() const = 0;

    /**
     * @brief Unique Component id for implementation type.
     */
    virtual uint32_t getComponentTypeId() const = 0;

    /**
     * @brief Entity instance this Component belongs to.
     */
    Entity& getEntity() const
    {
        return _entity;
    }
};

/**
 * @brief Base class for Entity Components, T must be Component implementation class.
 * @note Every T implementation must implement ComponentTypeName member.
 */
template <typename T>
class ComponentT : public ComponentBase {
public:
    ComponentT(Entity& entity)
        : ComponentBase(entity)
    {
    }

    /**
     * @brief Override getComponentTypeId to return T::GetComponentTypeId
     */
    uint32_t getComponentTypeId() const override
    {
        return ComponentT<T>::GetComponentTypeId();
    }

    /**
     * @brief Return component type name from T::ComponentTypeName.
     */
    virtual const std::string& getComponentTypeName() const override
    {
        return T::ComponentTypeName;
    }

    /**
     * @brief Returns a unique integer for every type T.
     */
    static uint32_t GetComponentTypeId()
    {
        static uint32_t componentTypeId = ComponentBase::ComponentTypeIdCounter++;
        return componentTypeId;
    }

    static const std::string ComponentTypeName;
};

template <typename T>
T* component_cast(ComponentBase* component)
{
    if (component == nullptr) {
        throw std::invalid_argument("component");
    }

    if (component->getComponentTypeId() != T::GetComponentTypeId()) {
        return nullptr;
    }
    auto result = dynamic_cast<T*>(component);

    if (result == nullptr) {
        throw std::logic_error(
            "ComponentTypeId matches requested Component type but dynamic_cast failed ?");
    }
    return result;
}
}
}

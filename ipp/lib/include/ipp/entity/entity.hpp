#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include <ipp/log.hpp>
#include <ipp/loop/message.hpp>
#include "component.hpp"

namespace ipp {
namespace entity {

/**
 * @brief Collection of ComponentBase derived types with a unique id in World.
 */
class Entity final : public NonCopyable {
private:
    World& _world;
    const uint32_t _id;
    const std::string _name;
    std::vector<std::unique_ptr<ComponentBase>> _componentBuffer;

    void dispatchComponentsModified();

public:
    Entity(World& world, uint32_t id, std::string name)
        : _world{world}
        , _id{id}
        , _name{std::move(name)}
    {
    }

    /**
     * @brief Create a new component of type T.
     */
    template <typename T, typename... Params>
    T* createComponent(Params&&... params)
    {
        auto componentTypeId = T::GetComponentTypeId();

        if (findComponent<T>()) {
            IVL_LOG_THROW_ERROR(std::runtime_error,
                                "Entity {} already contains required Component type {}", _name,
                                componentTypeId);
        }

        auto component = std::make_unique<T>(*this, std::forward<Params>(params)...);
        auto result = component.get();
        _componentBuffer.push_back(std::move(component));
        dispatchComponentsModified();
        return result;
    }

    /**
     * @brief Find Component of type T in Entity or nullptr if no Component found.
     */
    template <typename T>
    T* findComponent() const
    {
        auto component = findComponent(T::GetComponentTypeId());
        if (component == nullptr) {
            return nullptr;
        }
        return dynamic_cast<T*>(component);
    }

    /**
     * @brief Find ComponentBase instance with specified componentTypeId.
     */
    ComponentBase* findComponent(uint32_t componentTypeId) const;

    /**
     * @brief Find ComponentBase instance with specified componentTypeName.
     */
    ComponentBase* findComponent(const std::string& componentTypeName) const;

    /**
     * @brief Remove ComponentBase instance from Entity.
     */
    void removeComponent(ComponentBase* component);

    /**
     * @brief Owning World instance.
     */
    World& getWorld() const
    {
        return _world;
    }

    /**
     * @brief Entity associated unique id in World.
     */
    uint32_t getId() const
    {
        return _id;
    }

    /**
     * @brief Entity associated uinque name in World.
     */
    const std::string& getName() const
    {
        return _name;
    }

    /**
     * @brief Buffer used to store entity components.
     */
    const std::vector<std::unique_ptr<ComponentBase>>& getComponentBuffer() const
    {
        return _componentBuffer;
    }
};
}
}

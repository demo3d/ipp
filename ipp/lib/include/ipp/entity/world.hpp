#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include <ipp/loop/messageloop.hpp>
#include "entity.hpp"
#include "entityfilter.hpp"
#include "worldentityobserver.hpp"

namespace ipp {
namespace entity {

/**
 * @brief Container of Entity objects that watches all Entities for component add/remove events.
 */
class World : public ipp::NonCopyable {
public:
    friend class Entity;

private:
    std::unordered_map<uint32_t, std::unique_ptr<Entity>> _entities;
    std::vector<std::unique_ptr<WorldEntityObserver>> _entityObservers;
    uint32_t _maxEntityId;

    /**
     * @brief Called by Entity to notify parent World that it's Components have been updated.
     */
    void onEntityComponentsModified(Entity& entity);

public:
    World()
        : _maxEntityId{0}
    {
    }

    /**
     * @brief Create a new Entity object with a unique id and name string.
     */
    Entity* createEntity(uint32_t id, std::string entityName);

    /**
     * @brief Remove an existing Entity from World.
     * @return false if entity with id not found, true otherwise
     */
    bool removeEntity(uint32_t id);

    /**
     * @brief Get Entity with specific id.
     */
    Entity* findEntity(uint32_t id) const
    {
        auto entityIt = _entities.find(id);
        if (entityIt == _entities.end()) {
            return nullptr;
        }
        else {
            return entityIt->second.get();
        }
    }

    /**
     * @brief Get Entity with specific name, returns nullptr if no Entity with name found.
     * @note Always does a linear search over entities.
     */
    Entity* findEntity(const std::string& name) const;

    /**
     * @brief Create a new WorldEntityObserver instance and return a pointer to it.
     *
     * Entity group is owned by World and will immediately be populated with existing entities
     * that match entity group filters.
     */
    template <typename T, typename... Params>
    T* createEntityObserver(Params&&... params)
    {
        auto observer = std::make_unique<T>(*this, std::forward<Params>(params)...);

        for (auto& entity : _entities) {
            auto ptr = static_cast<entity::WorldEntityObserver*>(observer.get());
            ptr->onEntityComponentsModified(*entity.second);
        }

        auto result = observer.get();
        _entityObservers.push_back(std::move(observer));
        return result;
    }

    /**
     * @brief Remove EntityGroup from World.
     */
    bool removeEntityObserver(WorldEntityObserver* entityGroup);

    /**
     * @brief Highest entity id registered with World.
     * Never decreases even on entity removal.
     */
    uint32_t getMaxEntityId() const
    {
        return _maxEntityId;
    }

    /**
     * @brief Entity collection indexed by Entity Id.
     */
    const std::unordered_map<uint32_t, std::unique_ptr<Entity>>& getEntities() const
    {
        return _entities;
    }

    /**
     * @brief Returns a vector of Entity pointers that match EntityFilter.
     * EntityFilter is a type with a static "bool match(Entity&)" method.
     */
    template <typename EntityFilter>
    std::vector<Entity*> filterEntities() const
    {
        std::vector<Entity*> result;
        for (auto& entity : _entities) {
            if (EntityFilter::match(*entity.second)) {
                result.push_back(entity.second.get());
            }
        }
        return result;
    }

    /**
     * @brief Return a vector of entities that have all required component types.
     */
    template <typename... ComponentTypes>
    std::vector<Entity*> filterEntitiesWithComponents() const
    {
        std::vector<Entity*> result;
        for (auto& entity : _entities) {
            if (ContainsAllComponents<ComponentTypes...>::match(*entity.second)) {
                result.push_back(entity.second.get());
            }
        }
        return result;
    }

    /**
     * @brief All World entity groups.
     */
    const std::vector<std::unique_ptr<WorldEntityObserver>>& getEntityObservers() const
    {
        return _entityObservers;
    }
};
}
}

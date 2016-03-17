#include <ipp/entity/world.hpp>
#include <ipp/log.hpp>

using namespace std;
using namespace ipp::entity;

void World::onEntityComponentsModified(Entity& entity)
{
    for (auto& observer : _entityObservers) {
        observer->onEntityComponentsModified(entity);
    }
}

Entity* World::createEntity(uint32_t id, string entityName)
{
    if (id == 0) {
        IVL_LOG_THROW_ERROR(invalid_argument, "Entity ID cannot be 0");
    }

    if (entityName.length() == 0) {
        IVL_LOG_THROW_ERROR(invalid_argument, "Entity name cannot be empty");
    }

    if (_entities.find(id) != _entities.end()) {
        IVL_LOG_THROW_ERROR(runtime_error, "Entity : {} with same id : {} already exists in World",
                            entityName, id);
    }

    auto existing = findEntity(entityName);
    if (existing != nullptr) {
        IVL_LOG_THROW_ERROR(runtime_error, "Entity with same name : {} (requested id : {}, "
                                           "existing id : {}) already exists in World",
                            entityName, id, existing->getId());
    }

    if (_maxEntityId < id) {
        _maxEntityId = id;
    }

    auto entity = make_unique<Entity>(*this, id, move(entityName));
    auto result = entity.get();

    _entities.emplace(id, move(entity));

    for (auto& observer : _entityObservers) {
        observer->onWorldEntityCreated(*result);
    }

    return result;
}

bool World::removeEntity(uint32_t id)
{
    auto it = _entities.find(id);
    if (it == _entities.end()) {
        return false;
    }

    for (auto& observer : _entityObservers) {
        observer->onWorldEntityRemoving(*it->second);
    }

    _entities.erase(it);

    return true;
}

Entity* World::findEntity(const string& name) const
{
    auto it = find_if(_entities.begin(), _entities.end(),
                      [&name](auto& entity) { return entity.second->getName() == name; });
    if (it == _entities.end()) {
        return nullptr;
    }
    else {
        return it->second.get();
    }
}

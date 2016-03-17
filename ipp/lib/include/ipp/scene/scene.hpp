#pragma once

#include <ipp/shared.hpp>
#include <ipp/entity/world.hpp>

namespace ipp {
namespace scene {

/**
 * @brief Resource deserializing a Scene instance for
 */
class Scene final {
private:
    Context& _context;
    loop::MessageLoop _messageLoop;
    entity::World _world;
    std::string _resourcePath;

public:
    Scene(Context& context, std::unique_ptr<resource::ResourceBuffer> data);

    /**
     * @brief Post scene animation update message and update message loop.
     */
    void update();

    /**
     * @brief Scene MessageLoop
     */
    loop::MessageLoop& getMessageLoop()
    {
        return _messageLoop;
    }

    /**
     * @brief Scene MessageLoop (const access)
     */
    const loop::MessageLoop& getMessageLoop() const
    {
        return _messageLoop;
    }

    /**
     * @brief Scene collection of Entities
     */
    entity::World& getWorld()
    {
        return _world;
    }

    /**
     * @brief Scene collection of Entities (const access)
     */
    const entity::World& getWorld() const
    {
        return _world;
    }

    /**
     * @brief Resource path from which the scene was created.
     */
    const std::string& getResourcePath() const
    {
        return _resourcePath;
    }

    /**
     * @brief Parent Context instance.
     */
    Context& getContext()
    {
        return _context;
    }
};
}
}

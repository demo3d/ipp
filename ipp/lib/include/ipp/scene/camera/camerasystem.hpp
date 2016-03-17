#pragma once

#include <ipp/shared.hpp>
#include <ipp/entity/entitygroup.hpp>
#include "camerausercontrolledsystem.hpp"
#include "cameranodesystem.hpp"

namespace ipp {
namespace scene {
namespace camera {

/**
 * @brief Scene Loop system that updates all Node components attached to root node.
 */
class CameraSystem final : public ::ipp::loop::SystemT<CameraSystem> {
public:
    /**
     * @brief Command used to change CameraSystem active camera type
     */
    typedef loop::CommandT<CameraType> SetActiveTypeCommand;

    /**
     * @brief Event dispatched after CameraSytem active camera has been changed.
     */
    typedef loop::EventT<CameraType> ActiveTypeUpdatedEvent;

private:
    Camera* _activeCamera;
    CameraUserControlledSystem* _userControlledCamera;
    CameraNodeSystem* _nodeCamera;

    /**
     * @brief
     */
    std::vector<SystemBase*> initialize() override;

    /**
     * @brief
     */
    void onMessage(const loop::Message& message) override;

public:
    CameraSystem(loop::MessageLoop& messageLoop);

    /**
     * @brief Currently active scene camera.
     */
    Camera& getActiveCamera() const
    {
        return *_activeCamera;
    }
};
}
}
}

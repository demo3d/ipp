#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include <ipp/resource/package.hpp>

namespace ipp {
namespace render {

/**
 * @brief A collection of bones with consistent indexes and bind poses used for skinning.
 */
class Armature final : public resource::SharedResourceT<Armature> {
public:
    /**
     * @brief Bone id, name, parent and bind pose data.
     */
    class Bone final {
    public:
        struct Pose {
            Pose();
            Pose(glm::vec3 translation, glm::quat rotation, float scale);

            operator glm::mat4() const;

            glm::vec3 translation;
            glm::quat rotation;
            float scale;
        };

    private:
        std::string _name;
        size_t _index;
        int _parentIndex;
        bool _isDeform;
        Pose _bindPose;
        glm::mat4 _inverseBindPose;

    public:
        Bone(std::string name,
             size_t index,
             int parentIndex,
             bool isDeform,
             const glm::vec3& bindTranslation,
             const glm::quat& bindRotation,
             float bindScale);

        const std::string& getName() const
        {
            return _name;
        }

        bool isDeform() const
        {
            return _isDeform;
        }

        size_t getIndex() const
        {
            return _index;
        }

        int getParentIndex() const
        {
            return _parentIndex;
        }

        const Pose& getBindPose() const
        {
            return _bindPose;
        }

        const glm::mat4& getInverseBindPose() const
        {
            return _inverseBindPose;
        }
    };

private:
    std::vector<Bone> _bones;

public:
    Armature(std::unique_ptr<::ipp::resource::ResourceBuffer> data);

    /**
     * @brief Armature bone array.
     */
    const std::vector<Bone>& getBones() const
    {
        return _bones;
    }
};
}
}

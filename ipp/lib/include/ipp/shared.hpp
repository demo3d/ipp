#pragma once

#include <stdexcept>
#include <memory>
#include <array>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <cmath>
#include <functional>
#include <algorithm>
#include <typeinfo>

#ifndef IVL_LOGGING_DISABLED
#define FMT_HEADER_ONLY
#include <cppformat/format.h>
#endif

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <json.hpp>

using json = nlohmann::json;

namespace ipp {

// define Milliseconds as std::duration backed by a int32_t field
// because emscripten does not like 64 bit ints and we don't need the extra precision
//
// NOTE: This type should not be used to represent long time values, it's only use is
//       representation of application/scene durations like animation timespans
//       (int32_t can represent ~20 days in milliseconds).
typedef std::chrono::duration<int32_t, std::milli> Milliseconds;

// resource manager
namespace resource {
class ResourceBuffer;
class ResourceBufferMemory;

class Package;
class PackageFileSystem;
class PackageIPPArchive;

class SharedResourceBase;

class ResourceManager;
}

// message loop
namespace loop {
class SystemBase;
template <typename T>
class SystemT;

class Message;
class Event;
class Command;

class MessageLoop;
}

// entity-component system
namespace entity {
class Entity;
class WorldEntityObserver;
class World;

class ComponentBase;
template <typename T>
class ComponentT;
}

// OpenGL wrappers and render system
namespace render {
namespace gl {
class Texture;
class Texture2D;
class TextureCube;

class ShaderProgram;
}

class Effect;
class MaterialBuffer;
class Mesh;
}

// Top level scene manager
namespace scene {

// scene rendering
namespace render {
class MaterialEffect;
class Material;

class ArmatureComponent;

class Renderable;
class RenderableComponent;

class RenderPass;
class RenderPassContext;

class RenderSystem;
}

// scene node hierarchy/transform system
namespace node {
class Node;
class NodeComponent;
class NodeSystem;
}

// scene animation system
namespace animation {
template <typename T>
class ConstKeyFrameT;
class FloatKeyFrame;

class Channel;
class Action;

class Track;
template <typename T>
class Sequence;

class Marker;

class AnimationSystem;
}

// scene camera
namespace camera {
class Camera;

class CameraUserControlledSystem;

class CameraNodeComponent;
class CameraNodeSystem;

class CameraSystem;
}

class Scene;
}

class NonCopyable;
class Context;
}

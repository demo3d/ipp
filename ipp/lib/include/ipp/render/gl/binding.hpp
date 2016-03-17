#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>

namespace ipp {
namespace render {
namespace gl {

/**
 * @brief Binding class uses RAII to bind/unbind GL objects to active GL context
 *
 * Valid T types must expose :
 *
 *    bool isBound() const
 *    void bind() const
 *    void unbind() const
 *
 * Classes in gl namespace implement those methods as private members and add Binding<T>
 * as a friend class to force users to use Binding type.
 */
template <typename T>
class Binding final : public NonCopyable {
public:
    friend class RenderTarget;

private:
    const T* _target;

public:
    /**
     * @brief Acquire a new binding on target by calling target->bind and storing reference to
     * target
     */
    Binding(const T& target)
        : _target{&target}
    {
        _target->bind();
    }

    /**
     * @brief Move constructor - will invalidate other binding instance and move target to this
     * instance
     */
    Binding(Binding<T>&& other)
        : _target{other._target}
    {
        other._target = nullptr;
    }

    /**
     * @brief Release binding target
     */
    ~Binding()
    {
        if (_target != nullptr) {
            _target->unbind();
        }
    }

    /**
     * @brief Move assignment
     */
    Binding& operator=(Binding<T>&& other)
    {
        assert(other._target != nullptr);

        _target = other._target;
        other._target = nullptr;

        return *this;
    }

    /**
     * @brief Evaluates to isReferenceValid
     */
    operator bool() const
    {
        return isReferenceValid();
    }

    /**
     * @brief True if reference to target is not null (moved out of this object)
     */
    bool isReferenceValid() const
    {
        return _target != nullptr;
    }

    /**
     * @brief Binding target T instance bound to context by this object
     * @note If not isReferenceValid null reference will be returned (assert check in debug mode).
     */
    T& getTarget() const
    {
        assert(isReferenceValid());
        return *_target;
    }

    /**
     * @brief Assert that reference is valid and that target is bound in context
     * @note Does nothing in release build
     */
    void assertBound() const
    {
#ifdef IVL_DEBUG_BUILD
        assert(isReferenceValid());
        assert(_target->isBound());
#endif
    }
};
}
}
}
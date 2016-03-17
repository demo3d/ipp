#pragma once

#include <ipp/shared.hpp>
#include <ipp/log.hpp>
#include <ipp/noncopyable.hpp>
#include "gl/uniform.hpp"
#include "effect.hpp"

namespace ipp {
namespace render {

/**
 * @brief Memory buffer for a bound Effect instance.
 */
class MaterialBuffer {
private:
    std::shared_ptr<Effect> _effect;
    std::vector<uint8_t> _buffer;

public:
    MaterialBuffer(std::shared_ptr<Effect> effect)
        : _effect{std::move(effect)}
        , _buffer(_effect->getBufferSize())
    {
    }

    /**
     * @brief Copy constructor allows MaterialBuffer to initialize from existing buffer state.
     */
    MaterialBuffer(const MaterialBuffer& other)
        : _effect{other._effect}
        , _buffer{other._buffer}
    {
    }

    /**
     * @brief Disable copy operator, only copy constructor/initialization is supported.
     */
    void operator=(const MaterialBuffer& other) = delete;

    /**
     * @brief Read variable values from buffer to target.
     * Values are cast from UniformBufferTypeT to T.
     * If variable pointer is null function throws a runtime_error.
     */
    template <typename T>
    void getVariableValue(const Effect::Variable* variable, size_t count, T* target) const
    {
        if (variable == nullptr) {
            IVL_LOG_THROW_ERROR(std::invalid_argument,
                                "Cannot set variable value  when variable is null.");
        }

        if (variable->getUniformVariable().getType() != gl::UniformBufferType<T>::TypeEnum) {
            IVL_LOG_THROW_ERROR(std::invalid_argument,
                                "Invalid uniform : {0} type requested : {1:0X} expected : {2:0X}",
                                variable->getUniformVariable().getName(),
                                static_cast<int>(gl::UniformBufferType<T>::TypeEnum),
                                static_cast<int>(variable->getUniformVariable().getType()));
        }
        auto source = reinterpret_cast<const gl::UniformBufferTypeT<T>*>(
            _buffer.data() + variable->getBufferOffset());

        if (count > variable->getUniformVariable().getCount()) {
            IVL_LOG_THROW_ERROR(
                std::invalid_argument,
                "Invalid number of uniform : {0} values requested : {1} maximum : {2}",
                variable->getUniformVariable().getName(), count,
                variable->getUniformVariable().getCount());
        }

        for (size_t i = 0; i < count; ++i) {
            target[i] = gl::uniform_cast<T>(source[i]);
        }
    }

    /**
     * @brief Write uniform values from source to buffer.
     * Values are cast from T to UniformBufferTypeT.
     * If variable pointer is null returns immediately.
     */
    template <typename T>
    void setVariableValue(const Effect::Variable* variable, size_t count, const T* source)
    {
        if (variable == nullptr) {
            return;
        }

        if (variable->getUniformVariable().getType() != gl::UniformBufferType<T>::TypeEnum) {
            IVL_LOG_THROW_ERROR(std::invalid_argument,
                                "Invalid uniform : {0} type requested : {1:0X} expected : {2:0X}",
                                variable->getUniformVariable().getName(),
                                static_cast<int>(gl::UniformBufferType<T>::TypeEnum),
                                static_cast<int>(variable->getUniformVariable().getType()));
        }
        auto target = reinterpret_cast<gl::UniformBufferTypeT<T>*>(_buffer.data() +
                                                                   variable->getBufferOffset());

        if (count > variable->getUniformVariable().getCount()) {
            IVL_LOG_THROW_ERROR(
                std::invalid_argument,
                "Invalid number of uniform : {0} values requested : {1} maximum : {2}",
                variable->getUniformVariable().getName(), count,
                variable->getUniformVariable().getCount());
        }

        for (size_t i = 0; i < count; ++i) {
            target[i] = gl::uniform_cast<T>(source[i]);
        }
    }

    /**
     * @brief Untyped variable buffer memory access.
     */
    uint8_t* getVariableBuffer()
    {
        return _buffer.data();
    }

    /**
     * @brief Untyped variable const buffer memory access.
     */
    const uint8_t* getVariableBuffer() const
    {
        return _buffer.data();
    }

    /**
     * @brief Material Effect.
     */
    std::shared_ptr<Effect> getEffect() const
    {
        return _effect;
    }
};
}
}

#pragma once

#include <ipp/shared.hpp>
#include <ipp/noncopyable.hpp>
#include "entity.hpp"

namespace ipp {
namespace entity {

/**
 * @brief Matches Entity if all ComponentTypes are present in it.
 */
template <typename... ComponentTypes>
struct ContainsAllComponents;

template <>
struct ContainsAllComponents<> {
    static bool match(Entity& entity)
    {
        return true;
    }
};

template <typename First, typename... Rest>
struct ContainsAllComponents<First, Rest...> {
    static bool match(Entity& entity)
    {
        return entity.findComponent<First>() != nullptr &&
               ContainsAllComponents<Rest...>::match(entity);
    }
};

/**
 * @brief Matches Entity if none of ComponentTypes are present in it.
 */
template <typename... ComponentTypes>
struct ContainsNoComponents;

template <>
struct ContainsNoComponents<> {
    static bool match(Entity& entity)
    {
        return true;
    }
};

template <typename First, typename... Rest>
struct ContainsNoComponents<First, Rest...> {
    static bool match(Entity& entity)
    {
        return entity.findComponent<First>() == nullptr &&
               ContainsNoComponents<Rest...>::match(entity);
    }
};

/**
 * @brief Matches any entity.
 */
struct AnyEntity {
    static bool match(Entity& entities)
    {
        return true;
    }
};

/**
 * @brief Combines multiple Filter types and returns true if all of them match Entity.
 */
template <typename... Filters>
struct CombineFilters;

template <>
struct CombineFilters<> {
    static bool match(Entity& entity)
    {
        return true;
    }
};

template <typename First, typename... Rest>
struct CombineFilters<First, Rest...> {
    static bool match(Entity& entity)
    {
        return First::match(entity) && CombineFilters<Rest...>::match(entity);
    }
};
}
}

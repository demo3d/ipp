#pragma once

#include <ipp/shared.hpp>

namespace ipp {

/**
 * @brief Log message importance levels.
 */
enum class LogLevel { Trace = 0, Info, Warning, Error };

/**
 * @brief String representation of LogLevel enum value.
 * Returns lowercase string of level value enum meber name.
 */
const std::string& GetLevelString(LogLevel level);

/**
 * @brief Logs message to logging output and returns message string for convinience.
 */
std::string LogMessage(LogLevel level, std::string message);

#ifndef IVL_LOGGING_DISABLED
/**
 * @brief log function convenience macro
 *
 * Prepends LogLevel:: to LEVEL argument.
 * Passes the rest to fmt::format to get log message string.
 */
#define IVL_LOG(LEVEL, ...) ::ipp::LogMessage(::ipp::LogLevel::LEVEL, fmt::format(__VA_ARGS__))

/**
 * @brief Log Error message and use the message to construct and throw exception of specified TYPE
 */
#define IVL_LOG_THROW_ERROR(TYPE, ...)                                                             \
    throw TYPE(::ipp::LogMessage(::ipp::LogLevel::Error, fmt::format(__VA_ARGS__)))
#else
#define IVL_LOG(LEVEL, FORMAT_STRING, ...) FORMAT_STRING
#define IVL_LOG_THROW_ERROR(TYPE, FORMAT_STRING, ...) throw TYPE(FORMAT_STRING)
#endif
}

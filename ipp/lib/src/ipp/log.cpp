#ifndef IVL_LOGGING_DISABLED

#include <ipp/log.hpp>
#include <iostream>
#include <iomanip>

using namespace std;

namespace ipp {

const string& GetLevelString(LogLevel level)
{
    static string trace = "trace";
    static string info = "info";
    static string warning = "warning";
    static string error = "error";

    switch (level) {
        case LogLevel::Trace:
            return trace;
        case LogLevel::Info:
            return info;
        case LogLevel::Warning:
            return warning;
        case LogLevel::Error:
            return error;
    }
}

string LogMessage(LogLevel level, string message)
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    cout << std::put_time(std::localtime(&in_time_t), "%H:%M:%S:") << GetLevelString(level) << ": "
         << message << endl;

    return message;
}
}

#endif

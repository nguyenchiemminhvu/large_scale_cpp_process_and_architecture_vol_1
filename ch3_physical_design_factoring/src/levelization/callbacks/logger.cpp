// logger.cpp
#ifndef INCLUDED_LOGGER
#include <logger.h>
#endif

Logger::Logger()
: d_callback(nullptr)
, d_userData(nullptr)
, d_minSeverity(k_INFO)
, d_messageCount(0)
{
}

void Logger::setCallback(Callback cb, void* userData)
{
    d_callback = cb;
    d_userData = userData;
}

void Logger::setMinSeverity(int severity)
{
    d_minSeverity = severity;
}

void Logger::log(const std::string& message, int severity)
{
    if (severity < d_minSeverity) return;
    ++d_messageCount;
    if (d_callback) {
        d_callback(message.c_str(), severity, d_userData);
    }
}

int Logger::messageCount() const { return d_messageCount; }
bool Logger::hasCallback() const { return d_callback != nullptr; }

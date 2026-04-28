// logger.h                                                           -*-C++-*-
//
// Demonstrates DATA CALLBACK levelization.
// Logger accepts a function pointer + void* (C-style data callback).

#ifndef INCLUDED_LOGGER
#define INCLUDED_LOGGER

#include <string>

/// A simple logger that dispatches log messages through a data callback.
///
/// DATA CALLBACK pattern: The logger accepts a C-style function pointer
/// plus a 'void*' user data pointer. This allows callers to attach any
/// context they need without Logger knowing about their types.
///
/// This is the simplest (and oldest) form of callback — no virtual dispatch,
/// no std::function overhead. Very low-level, ABI-stable.
class Logger {
  public:
    /// Type for the log callback.
    /// Parameters: message, severity, userData
    using Callback = void(*)(const char* message, int severity, void* userData);

    enum Severity {
        k_DEBUG   = 0,
        k_INFO    = 1,
        k_WARNING = 2,
        k_ERROR   = 3,
    };

  private:
    Callback d_callback;
    void*    d_userData;
    int      d_minSeverity;
    int      d_messageCount;

  public:
    Logger();

    // MANIPULATORS

    /// Sets the callback and associated user data.
    void setCallback(Callback cb, void* userData = nullptr);

    /// Sets the minimum severity level to log.
    void setMinSeverity(int severity);

    /// Logs a message at the given severity.
    void log(const std::string& message, int severity = k_INFO);

    // ACCESSORS
    int messageCount() const;
    bool hasCallback() const;
};

#endif  // INCLUDED_LOGGER

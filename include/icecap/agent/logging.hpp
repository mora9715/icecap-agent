#pragma once

#include <string>
#include <memory>

namespace spdlog {
    class logger;
}

namespace icecap::agent {

class Logger {
public:
    static Logger& getInstance();

    void initialize(const std::string& logFilePath = "");
    void shutdown();

    void trace(const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);


private:
    Logger() = default;
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string getDefaultLogPath() const;

    std::shared_ptr<spdlog::logger> m_logger;
    bool m_initialized = false;
};

// Convenience macros for easier logging - simplified for compilation
#define LOG_TRACE(msg) icecap::agent::Logger::getInstance().trace(msg)
#define LOG_DEBUG(msg) icecap::agent::Logger::getInstance().debug(msg)
#define LOG_INFO(msg) icecap::agent::Logger::getInstance().info(msg)
#define LOG_WARN(msg) icecap::agent::Logger::getInstance().warn(msg)
#define LOG_ERROR(msg) icecap::agent::Logger::getInstance().error(msg)
#define LOG_CRITICAL(msg) icecap::agent::Logger::getInstance().critical(msg)

} // namespace icecap::agent
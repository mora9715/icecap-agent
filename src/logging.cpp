#include "icecap/agent/logging.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <windows.h>
#include <filesystem>
#include <chrono>
#include <atomic>

namespace icecap::agent {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::initialize(const std::string& logFilePath) {
    if (m_initialized) {
        return;
    }

    try {
        std::string actualLogPath = logFilePath.empty() ? getDefaultLogPath() : logFilePath;

        // Ensure the directory exists
        std::filesystem::path logPath(actualLogPath);
        std::filesystem::create_directories(logPath.parent_path());

        // Create rotating file sink (max 5MB per file, keep 3 files)
        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            actualLogPath, 1024 * 1024 * 5, 3);
        file_sink->set_level(spdlog::level::trace);

        m_logger = std::make_shared<spdlog::logger>("icecap-agent", file_sink);

        // Set pattern: [timestamp] [level] [thread] message
        m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");

        // Set level based on build type
#ifdef _DEBUG
        m_logger->set_level(spdlog::level::debug);
#else
        m_logger->set_level(spdlog::level::info);
#endif

        // Enable automatic flushing for immediate log visibility
        spdlog::flush_on(spdlog::level::trace);
        spdlog::flush_every(std::chrono::seconds(1)); // Also flush every second

        // Register logger globally
        spdlog::register_logger(m_logger);

        m_initialized = true;

        LOG_INFO("Logging system initialized");
    }
    catch (const std::exception& ex) {
        // Fallback to MessageBox if logging initialization fails
        std::string error = "Failed to initialize logging system: " + std::string(ex.what()) +
                           "\nAttempted log path: " + (logFilePath.empty() ? getDefaultLogPath() : logFilePath);
        MessageBoxA(nullptr,
            error.c_str(),
            "Icecap Agent - Logging Error",
            MB_OK | MB_ICONERROR);
    }
}

void Logger::shutdown() {
    static std::atomic<bool> shutdown_called{false};

    // Prevent multiple shutdown calls
    if (shutdown_called.exchange(true)) {
        return;
    }

    try {
        if (m_initialized && m_logger) {
            LOG_INFO("Shutting down logging system");
            m_logger->flush();
            spdlog::shutdown(); // Properly shutdown spdlog
            m_logger.reset();
            m_initialized = false;
        }
    }
    catch (...) {
        // Ignore any exceptions during shutdown
    }
}

std::string Logger::getDefaultLogPath() const {
    // Default to %TEMP%\icecap-agent\icecap-agent.log
    char tempPath[MAX_PATH];
    DWORD result = GetTempPathA(MAX_PATH, tempPath);

    if (result == 0 || result > MAX_PATH) {
        // Fallback to current directory if temp path fails
        return ".\\icecap-agent.log";
    }

    std::filesystem::path logPath(tempPath);
    logPath /= "icecap-agent";
    logPath /= "icecap-agent.log";

    return logPath.string();
}

// Simple string message methods
void Logger::trace(const std::string& message) {
    if (m_initialized && m_logger) {
        m_logger->trace(message);
    }
}

void Logger::debug(const std::string& message) {
    if (m_initialized && m_logger) {
        m_logger->debug(message);
    }
}

void Logger::info(const std::string& message) {
    if (m_initialized && m_logger) {
        m_logger->info(message);
    }
}

void Logger::warn(const std::string& message) {
    if (m_initialized && m_logger) {
        m_logger->warn(message);
    }
}

void Logger::error(const std::string& message) {
    if (m_initialized && m_logger) {
        m_logger->error(message);
    }
}

void Logger::critical(const std::string& message) {
    if (m_initialized && m_logger) {
        m_logger->critical(message);
    }
}


}
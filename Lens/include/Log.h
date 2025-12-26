#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <memory>
#include <unordered_map>
#include <format>

namespace lens
{
    namespace _logs
    {
        class Log
        {
        public:
            static Log* const Instance()
            {
                static Log* instance = new Log();
                return instance;
            }

            spdlog::logger* get() const
            {
                return m_logger.get();
            }

        private:
            Log()
            {
                std::vector<spdlog::sink_ptr> sinks;

                auto sink1 = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                sinks.push_back(sink1);
                auto sink2 = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("Lens.log", 1024 * 1024 * 10, 100, false);
                sinks.push_back(sink2);

                m_logger = std::make_shared<spdlog::logger>("global", begin(sinks), end(sinks));

                m_logger->set_pattern("[%F %S %T] [%s:%#] [%^%l%$] : %v");
                m_logger->set_level(spdlog::level::trace);

                SPDLOG_LOGGER_INFO(m_logger, "Logger init successful");
            }

        private:
            std::shared_ptr<spdlog::logger> m_logger;
        };
    }
    inline _logs::Log* const Logger = _logs::Log::Instance();
}

#define LOG_INFO(...) (SPDLOG_LOGGER_INFO(::lens::Logger->get(), std::format(__VA_ARGS__)));
#define LOG_TRACE(...) (SPDLOG_LOGGER_TRACE(::lens::Logger->get(), std::format(__VA_ARGS__)));
#define LOG_DEBUG(...) (SPDLOG_LOGGER_DEBUG(::lens::Logger->get(), std::format(__VA_ARGS__)));
#define LOG_WARN(...) (SPDLOG_LOGGER_WARN(::lens::Logger->get(), std::format(__VA_ARGS__)));
#define LOG_ERROR(...) (SPDLOG_LOGGER_ERROR(::lens::Logger->get(), std::format(__VA_ARGS__)));
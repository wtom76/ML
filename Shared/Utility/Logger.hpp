#pragma once
#include <Shared/LibIncludes/IncludeSpdlog.h>
//#include <Shared/Utility/System.hpp>
//#include <Shared/Utility/ScopeLifeTime.hpp>

// TODO: now createConsoleLogger should be called before creation of any derivative of Logged with "log". Refactor.

namespace util
{
	//---------------------------------------------------------------------------------------------------------
	inline std::shared_ptr<spdlog::logger> create_console_logger(const std::string& name = "log")
	{
		try
		{
			std::shared_ptr<spdlog::logger> log = spdlog::get(name);
			if (log)
			{
				return log;
			}

			const std::string log_root = getLogRoot(mbcs(exeFileNameW()));
			CreateDirectoryA(log_root.c_str(), NULL);

			std::vector<spdlog::sink_ptr> sinks
			{
				std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>(),
				std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_root + name + ".log", 8 * 1024 * 1024, 12)
			};
			log = std::make_shared<spdlog::async_logger>(name, begin(sinks), end(sinks), spdlog::thread_pool(),
				spdlog::async_overflow_policy::block);
			log->set_level(spdlog::level::trace);
			log->set_pattern("%^%D %H:%M:%S:%e %v%$");
			log->set_error_handler([](const std::string& msg)
				{
					std::cout << "Console logger error: " << msg << std::endl;
				});
			spdlog::register_logger(log);

			log->info("Logger has been created");
			SPDLOG_LOGGER_TRACE(log, "SPDLOG_LEVEL_TRACE");
			SPDLOG_LOGGER_DEBUG(log, "SPDLOG_LEVEL_DEBUG");
			SPDLOG_LOGGER_INFO(log, "SPDLOG_LEVEL_INFO");

			return log;
		}
		catch (const std::exception& ex)
		{
			std::cout << "Failed to create logger: " << ex.what() << std::endl;
			throw;
		}
	}
	//---------------------------------------------------------------------------------------------------------
	inline std::shared_ptr<spdlog::logger> create_logger(const std::string& name,
		const std::string& pattern = "%v", size_t max_size = 8 * 1024 * 1024, size_t max_files = 12)
	{
		std::shared_ptr<spdlog::logger> log = spdlog::get(name);
		if (log)
		{
			return log;
		}

		const std::string log_root = util::getLogRoot(util::exeFileName());
		CreateDirectoryA(log_root.c_str(), NULL);
		std::vector<spdlog::sink_ptr> sinks
		{
			std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_root + name + ".log", max_size, max_files)
		};
		log = std::make_shared<spdlog::async_logger>(name, begin(sinks), end(sinks), spdlog::thread_pool(),
			spdlog::async_overflow_policy::block);
		log->set_level(spdlog::level::trace);
		log->set_pattern(pattern);
		log->set_error_handler([name](const std::string& msg)
			{
				std::cerr << name << " logger error: " << msg << std::endl;
			});
		spdlog::register_logger(log);

		return log;
	}

	//---------------------------------------------------------------------------------------------------------
	// An object should exist while logger lib is used
	class SpdlogAsyncInit : private ScopeLifeTime<void>
	{
	public:
		SpdlogAsyncInit()
			: ScopeLifeTime<void>(
				[]()
				{
					spdlog::init_thread_pool(8192, 1);
				},
				[]()
				{
					spdlog::apply_all([](std::shared_ptr<spdlog::logger> log){ log->flush(); });
					spdlog::shutdown();
				}
			)
		{}
	};

	//---------------------------------------------------------------------------------------------------------
	// Logged
	class Logged
	{
	protected:
		std::shared_ptr<spdlog::logger> log_;

	protected:
		explicit Logged(const std::string& name = "log", const std::string& pattern = std::string{})
			: log_(create_logger(name, pattern))
		{}
		explicit Logged(std::shared_ptr<spdlog::logger> log)
			: log_(log)
		{}
		~Logged()
		{
			log_->flush();
		}

		spdlog::logger* log() const noexcept { return log_.get(); }
	};

	//---------------------------------------------------------------------------------------------------------
	// slow log getter for exceptional cases
	inline std::shared_ptr<spdlog::logger> log() { return spdlog::get("log"); }
}

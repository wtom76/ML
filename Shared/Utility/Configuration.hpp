#pragma once

#include <string>
#include <Shared/LibIncludes/IncludeJson.h>

namespace util::config
{
	//---------------------------------------------------------------------------------------------------------
	inline std::filesystem::path file_path(const std::string& label)
	{
		return util::exePath().replace_extension("."s + label + ".json");
	}
	//---------------------------------------------------------------------------------------------------------
	template <class ConfigT>
	inline void load(ConfigT& config, const std::filesystem::path& file_path)
	{
		try
		{
			std::ifstream cfg_file(file_path);
			if (!cfg_file)
			{
				throw std::runtime_error(std::string("Can't open ") + file_path.string());
			}
			else
			{
				cfg_file >> config;
			}
		}
		catch (const std::exception& ex)
		{
			SPDLOG_LOGGER_CRITICAL(log(), "Default config {:s} is created due to {:s}.",
				file_path.string() + ".default", ex.what());
			std::ofstream(file_path.string() + ".default") << config;
			throw;
		}
	}
	//---------------------------------------------------------------------------------------------------------
	template <class ConfigT>
	inline void store(ConfigT& config, const std::filesystem::path& file_path)
	{
		std::ofstream cfg_file(file_path);
		if (!cfg_file)
		{
			throw std::runtime_error(std::string("Can't open ") + file_path.string());
		}
		else
		{
			cfg_file << config;
		}
	}
}

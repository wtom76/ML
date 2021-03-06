#pragma once
#include <chrono>
#include <time.h>

namespace util::time
{
	//-------------------------------------------------------------------------------------------------
	inline std::string to_date_string(std::chrono::system_clock::time_point tp)
	{
		std::time_t tp_c{std::chrono::system_clock::to_time_t(tp)};
		std::tm tp_tm{};
		localtime_s(&tp_tm, &tp_c);
		char buf[128];
		if (strftime(buf, sizeof buf, "%Y/%m/%d", &tp_tm))
		{
			return std::string(buf);
		}
		return {};
	}
}
#pragma once

// https://github.com/gabime/spdlog

#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/wincolor_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/fmt/ostr.h>

// In case of "Console logger error: async log: thread pool doens't exist anymore" probably
//    const util::SpdlogAsyncInit spdlog_init;
// is missed before log creation.

#pragma once

#if !defined(_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
	// boost_1_66_0/asio generates warnings
	#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#endif

#if !defined(SPDLOG_DEBUG_ON)
	#define SPDLOG_DEBUG_ON
#endif

#if !defined(SPDLOG_ACTIVE_LEVEL)
//
// enable/disable log calls at compile time according to global level.
//
// define SPDLOG_ACTIVE_LEVEL to one of those (before including spdlog.h):
// SPDLOG_LEVEL_TRACE,	- almost all calls with parameters
// SPDLOG_LEVEL_DEBUG,	- less verbose than TRACE, just events, that can help to detect bugs
// SPDLOG_LEVEL_INFO,	- release mode from here
// SPDLOG_LEVEL_WARN,
// SPDLOG_LEVEL_ERROR,
// SPDLOG_LEVEL_CRITICAL,
// SPDLOG_LEVEL_OFF
//
	#if defined (SPDLOG_DEBUG_ON)
		#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
		//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
	#else
		#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
	#endif
#endif

#if !defined(ENABLE_MICROTREND_LOG)
	//#define ENABLE_MICROTREND_LOG
#endif

// Orders books snapshots are logged
#if !defined(DEBUG_ORDER_BOOKS)
	//#define DEBUG_ORDER_BOOKS
#endif

// Log of parsed FIX
#if !defined(ENABLE_FIX_LOG)
	#define ENABLE_FIX_LOG
#endif

// Log of raw FIX incoming traffic
#if !defined(ENABLE_FIX_DUMP)
	//#define ENABLE_FIX_DUMP
#endif

#if !defined(DUMP_RAW_FAST_MSG)
	//#define DUMP_RAW_FAST_MSG
#endif

// Log decoded FAST message
#if !defined(LOG_FAST_MSG)
	//#define LOG_FAST_MSG
#endif

#if !defined(ENABLE_PROFILING)
	//#define ENABLE_PROFILING
#endif

#if !defined(CHECK_FAST_MSG_BOUNDS)
	#define CHECK_FAST_MSG_BOUNDS
#endif

#ifdef ENABLE_FIX_LOG
#define FIXLOG(logger, ...) logger->trace(__VA_ARGS__)
#else
#define FIXLOG(logger, ...)
#endif

// should be included in stdafx.h before include <afxwin.h>
//#if !defined(NOMINMAX)
//	#define NOMINMAX
//	#include <algorithm>
//	using std::min;
//	using std::max;
//#endif

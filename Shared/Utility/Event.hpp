#pragma once
#include <system_error>
#include <Shared/Utility/SysIncludes.hpp>
#include <Shared/Utility/Unicode.hpp>

namespace util
{
	using namespace std::literals;

	//-------------------------------------------------------------------------------------------------------
	class Event
	{
		HANDLE event_ = nullptr;

	public:
		Event(const Event&) = delete;
		Event& operator =(const Event&) = delete;
		Event(Event&& src) noexcept
			: event_(src.event_)
		{
			src.event_ = nullptr;
		}
		Event& operator =(Event&& src) noexcept
		{
			if (&src != this)
			{
				event_ = src.event_;
				src.event_ = nullptr;
			}
			return *this;
		}
	public:
		Event() : event_(CreateEvent(nullptr, FALSE, FALSE, nullptr))
		{
			if (!event_)
			{
				throw std::system_error(std::error_code(GetLastError(), std::system_category()), "Event: CreateEvent failed"s);
			}
		}
		explicit Event(const char* name) : event_(CreateEvent(nullptr, FALSE, FALSE, utf16(name).c_str()))
		{
			if (!event_)
			{
				throw std::system_error(std::error_code(GetLastError(), std::system_category()), "Event: CreateEvent failed"s);
			}
		}
		explicit Event(const wchar_t* name) : event_(CreateEvent(nullptr, FALSE, FALSE, name))
		{
			if (!event_)
			{
				throw std::system_error(std::error_code(GetLastError(), std::system_category()), "Event: CreateEvent failed"s);
			}
		}
		~Event()
		{
			if (event_)
			{
				CloseHandle(event_);
			}
		}
		void wait(DWORD millisec) const
		{
			const DWORD res = WaitForSingleObject(event_, millisec);
			if (res != WAIT_OBJECT_0)
			{
				throw std::system_error(std::error_code(GetLastError(), std::system_category()), "Event::wait failed"s);
			}
		}
		// return true if event was set, false in case of timeout, throws EventException otherwise
		bool wait_with_timeout(DWORD millisec) const
		{
			switch (WaitForSingleObject(event_, millisec))
			{
			case WAIT_OBJECT_0: return true;
			case WAIT_TIMEOUT: return false;
			default:
				throw std::system_error(std::error_code(GetLastError(), std::system_category()), "Event::wait failed"s);
			}
		}
		// Heavy call
		bool is_set() const
		{
			switch (WaitForSingleObject(event_, 0))
			{
			case WAIT_OBJECT_0: return true;
			case WAIT_TIMEOUT: return false;
			default:
				throw std::system_error(std::error_code(GetLastError(), std::system_category()), "Event::wait failed"s);
			}
		}
		void set()
		{
			if (!SetEvent(event_))
			{
				throw std::system_error(std::error_code(GetLastError(), std::system_category()), "Event::set failed"s);
			}
		}
		// don't close it!
		HANDLE native_handle() const noexcept { return event_; }
	};
}

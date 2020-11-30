#pragma once
#include <shared_mutex>
#include <mutex>

namespace util::mt
{
	//-------------------------------------------------------------------------------------------------
	class SharedMutex
	{
	// types
	public:
		using ReadLock = std::shared_lock<std::shared_mutex>;
		using WriteLock = std::unique_lock<std::shared_mutex>;
	// data
	private:
		mutable std::shared_mutex mtx_;
	// methods
	public:
		ReadLock read_lock() const noexcept
		{
			return ReadLock(mtx_);
		}
		WriteLock write_lock() noexcept
		{
			return WriteLock(mtx_);
		}
		void unsafe_read_lock() const noexcept
		{
			mtx_.lock_shared();
		}
		void unsafe_read_unlock() const noexcept
		{
			mtx_.unlock_shared();
		}
		void unsafe_write_lock() noexcept
		{
			mtx_.lock();
		}
		void unsafe_write_unlock() noexcept
		{
			mtx_.unlock();
		}
	};
	//-------------------------------------------------------------------------------------------------
	class Mutex
	{
	// types
	public:
		using ReadLock = std::unique_lock<std::mutex>;
		using WriteLock = std::unique_lock<std::mutex>;
	// data
	private:
		mutable std::mutex mtx_;
	// methods
	public:
		ReadLock read_lock() const noexcept
		{
			return ReadLock(mtx_);
		}
		WriteLock write_lock() noexcept
		{
			return WriteLock(mtx_);
		}
		void unsafe_read_lock() const noexcept
		{
			mtx_.lock();
		}
		void unsafe_read_unlock() const noexcept
		{
			mtx_.unlock();
		}
		void unsafe_write_lock() noexcept
		{
			mtx_.lock();
		}
		void unsafe_write_unlock() noexcept
		{
			mtx_.unlock();
		}
	};
	//-------------------------------------------------------------------------------------------------
	//class NamedMutex
	//{
	//// types
	//public:
	//	using ReadLock = NamedMutexGuard;
	//	using WriteLock = NamedMutexGuard;
	//// data
	//private:
	//	mutable NamedMutex mtx_;
	//// methods
	//public:
	//	MtNamedMutex(const char* name) noexcept : mtx_(name)
	//	{}
	//	MtNamedMutex(const wchar_t* name) noexcept : mtx_(name)
	//	{}
	//	ReadLock read_lock() const noexcept
	//	{
	//		return ReadLock(mtx_);
	//	}
	//	WriteLock write_lock() noexcept
	//	{
	//		return WriteLock(mtx_);
	//	}
	//	void unsafe_read_lock() const noexcept
	//	{
	//		mtx_.lock();
	//	}
	//	void unsafe_read_unlock() const noexcept
	//	{
	//		mtx_.unlock();
	//	}
	//	void unsafe_write_lock() noexcept
	//	{
	//		mtx_.lock();
	//	}
	//	void unsafe_write_unlock() noexcept
	//	{
	//		mtx_.unlock();
	//	}
	//};
	//-------------------------------------------------------------------------------------------------
	class CriticalSection
	{
	// types
	public:
		using ReadLock = std::unique_lock<CriticalSection>;;
		using WriteLock = std::unique_lock<CriticalSection>;;
	// data
	private:
		mutable CRITICAL_SECTION cs_;
	// methods
	public:
		CriticalSection() noexcept
		{
			InitializeCriticalSection(&cs_);
		}
		~CriticalSection() noexcept
		{
			DeleteCriticalSection(&cs_);
		}
		void lock() const noexcept
		{
			EnterCriticalSection(&cs_);
		}
		void unlock() const noexcept
		{
			LeaveCriticalSection(&cs_);
		}
		ReadLock read_lock() const noexcept
		{
			return ReadLock(const_cast<CriticalSection&>(*this));
		}
		WriteLock write_lock() noexcept
		{
			return WriteLock(const_cast<CriticalSection&>(*this));
		}
		void unsafe_read_lock() const noexcept
		{
			lock();
		}
		void unsafe_read_unlock() const noexcept
		{
			unlock();
		}
		void unsafe_write_lock() noexcept
		{
			lock();
		}
		void unsafe_write_unlock() noexcept
		{
			unlock();
		}
	};
	//-------------------------------------------------------------------------------------------------
	class None
	{
	// types
	public:
		class LockNone {};
		using ReadLock = LockNone;
		using WriteLock = LockNone;
	// data
	// methods
	public:
		ReadLock read_lock() const noexcept
		{
			return ReadLock();
		}
		WriteLock write_lock() noexcept
		{
			return WriteLock();
		}
		void unsafe_read_lock() const noexcept
		{
		}
		void unsafe_read_unlock() const noexcept
		{
		}
		void unsafe_write_lock() noexcept
		{
		}
		void unsafe_write_unlock() noexcept
		{
		}
	};
}

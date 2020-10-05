#pragma once
#include <functional>

namespace util
{
	//-------------------------------------------------------------------------------------------------------
	// class StartStopGuard
	//-------------------------------------------------------------------------------------------------------
	template <typename Data>
	class ScopeLifeTime
	{
	public:
		using Func = std::function<void(Data&)>;
	private:
		Data data_;
		Func ctor_func_;
		Func dtor_func_;

	public:
		ScopeLifeTime(Func ctor_func, Func dtor_func)
			: ctor_func_(std::move(ctor_func))
			, dtor_func_(std::move(dtor_func))
		{
			ctor_func_(data_);
		}
		~ScopeLifeTime()
		{
			dtor_func_(data_);
		}
	};
	//-------------------------------------------------------------------------------------------------------
	// class StartStopGuard<void>
	//-------------------------------------------------------------------------------------------------------
	template <>
	class ScopeLifeTime<void>
	{
	public:
		using Func = std::function<void()>;
	private:
		Func ctor_func_;
		Func dtor_func_;

	public:
		ScopeLifeTime(Func ctor_func, Func dtor_func)
			: ctor_func_(std::move(ctor_func))
			, dtor_func_(std::move(dtor_func))
		{
			ctor_func_();
		}
		~ScopeLifeTime() noexcept
		{
			try
			{
				dtor_func_();
			}
			catch (const std::exception&)
			{
			}
		}
	};
	//-------------------------------------------------------------------------------------------------------
	// class StartStopGuard
	//-------------------------------------------------------------------------------------------------------
	template <class T>
	class StartStopGuard
	{
		T& object_;
	public:
		//-------------------------------------------------------------------------------------------------------
		explicit StartStopGuard(T& object)
			: object_(object)
		{
			object_.start();
		}
		//-------------------------------------------------------------------------------------------------------
		template <typename... Args>
		explicit StartStopGuard(T& object, Args&... args)
			: object_(object)
		{
			object_.start(args...);
		}
		//-------------------------------------------------------------------------------------------------------
		~StartStopGuard()
		{
			object_.stop();
		}

		//-------------------------------------------------------------------------------------------------------
		StartStopGuard() = delete;
		StartStopGuard(const StartStopGuard&) = delete;
		StartStopGuard& operator =(const StartStopGuard&) = delete;
		StartStopGuard(StartStopGuard&&) = delete;
		StartStopGuard& operator =(StartStopGuard&&) = delete;
	};
	//-------------------------------------------------------------------------------------------------------
	// class LockedRef
	//-------------------------------------------------------------------------------------------------------
	template <class Data>
	class LockedRef
	{
	public:
		Data& data_;
		typename Data::WriteLock lock_;

	public:
		LockedRef(Data& data)
			: data_(data)
			, lock_(data.writeLock())
		{}

		LockedRef() = delete;
		LockedRef(const LockedRef&) = delete;
		LockedRef& operator =(const LockedRef&) = delete;
		LockedRef(LockedRef&&) = delete;
		LockedRef& operator =(LockedRef&&) = delete;
	};
	//-------------------------------------------------------------------------------------------------------
	// class LockedRefConst
	//-------------------------------------------------------------------------------------------------------
	template <class Data>
	class LockedRefConst
	{
	public:
		const Data& data_;
		typename Data::ReadLock lock_;

	public:
		LockedRefConst(const Data& data)
			: data_(data)
			, lock_(data.readLock())
		{}

		LockedRefConst() = delete;
		LockedRefConst(const LockedRefConst&) = delete;
		LockedRefConst& operator =(const LockedRefConst&) = delete;
		LockedRefConst(LockedRefConst&&) = delete;
		LockedRefConst& operator =(LockedRefConst&&) = delete;
	};
}
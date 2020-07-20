#include "stdafx.h"
#include "TrainingSupervision.h"
#include "TrainingTask.h"

//----------------------------------------------------------------------------------------------------------
TrainingSupervision::~TrainingSupervision()
{
	stop();
}

//----------------------------------------------------------------------------------------------------------
void TrainingSupervision::_run_thread()
{
	for (;;)
	{
		task_ptr task;
		{
			std::unique_lock<std::mutex> lock{task_queue_mutex_};
			task_queue_cv_.wait(lock, [this]{ return !task_queue_.empty() || !run_; });
			if (!run_)
			{
				return;
			}
			task = std::move(task_queue_.front());
			task_queue_.pop_front();
		}
		try
		{
			task->run();
			if (run_)
			{
				if (task_ptr new_task{task->create_next()})
				{
					_add_task(std::move(new_task));
				}
			}
		}
		catch (const std::exception& /*ex*/)
		{
			// TODO: handle exception
		}
	}
}
//----------------------------------------------------------------------------------------------------------
void TrainingSupervision::_add_task(task_ptr task)
{
	{
		std::unique_lock<std::mutex> lock{task_queue_mutex_};
		task_queue_.emplace_back(std::move(task));
	}
	task_queue_cv_.notify_one();
}
//----------------------------------------------------------------------------------------------------------
void TrainingSupervision::start()
{
	run_ = true;
	const size_t thread_count = std::thread::hardware_concurrency();
	thread_pool_.reserve(thread_count);
	for (size_t i = 0; i < thread_count; ++i)
	{
		thread_pool_.emplace_back(std::thread([this]{ _run_thread(); }));
	}
}
//----------------------------------------------------------------------------------------------------------
void TrainingSupervision::stop()
{
	{
		std::unique_lock<std::mutex> lock{task_queue_mutex_};
		if (run_)
		{
			run_ = false;
			for (task_ptr& task : task_queue_)
			{
				task->cancel();
			}
			task_queue_cv_.notify_all();
		}
	}

	for (std::thread& thread_in_pool : thread_pool_)
	{
		thread_in_pool.join();
	}
}

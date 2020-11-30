#include "stdafx.h"
#include "TrainingSupervision.h"
#include "TrainingTask.h"

//----------------------------------------------------------------------------------------------------------
TrainingSupervision::~TrainingSupervision()
{
	stop();
}

//----------------------------------------------------------------------------------------------------------
// If there is no enqueued task, wait for task to appear in queue.
// Take first task from queue and run it.
void TrainingSupervision::_run_thread()
{
	for (;;)
	{
		task_ptr task;
		{
			unique_lock<mutex> lock{task_queue_mutex_};
			task_queue_cv_.wait(lock, [this]{ return !task_queue_.empty() || !run_; });
			if (!run_)
			{
				return;
			}
			task = move(task_queue_.front());
			task_queue_.pop_front();
		}
		try
		{
			task->run();
			if (run_)
			{
				if (task_ptr new_task{task->create_next()})
				{
					_add_task(move(new_task));
				}
			}
		}
		catch (const exception& /*ex*/)
		{
			// TODO: handle exception
		}
	}
}
//----------------------------------------------------------------------------------------------------------
void TrainingSupervision::_add_task(task_ptr task)
{
	{
		unique_lock<mutex> lock{task_queue_mutex_};
		task_queue_.emplace_back(move(task));
	}
	task_queue_cv_.notify_one();
}
//----------------------------------------------------------------------------------------------------------
void TrainingSupervision::init_tasks(shared_ptr<TrainingTask> initial_task)
{
	assert(!run_);
	assert(thread_pool_.empty());

	task_queue_.clear();
	task_queue_.emplace_back(move(initial_task));
}
//----------------------------------------------------------------------------------------------------------
// will be 1 taks more that thread_count
void TrainingSupervision::start()
{
	run_ = true;
	const size_t thread_count = thread::hardware_concurrency();
	assert(task_queue_.size() == 1);
	for (size_t i = 1; i < thread_count; ++i)
	{
		task_queue_.emplace_back(task_queue_.back()->create_next());
	}
	assert(thread_pool_.empty());
	thread_pool_.reserve(thread_count);
	for (size_t i = 0; i < thread_count; ++i)
	{
		thread_pool_.emplace_back(thread([this]{ _run_thread(); }));
	}
}
//----------------------------------------------------------------------------------------------------------
void TrainingSupervision::stop()
{
	{
		unique_lock<mutex> lock{task_queue_mutex_};
		if (run_)
		{
			run_ = false;
			task_queue_cv_.notify_all();
		}
	}

	for (thread& thread_in_pool : thread_pool_)
	{
		thread_in_pool.join();
	}
	thread_pool_.clear();
}

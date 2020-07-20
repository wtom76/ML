#pragma once
#include <atomic>
#include <deque>
#include <vector>
#include <future>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <memory>

// https://stackoverflow.com/questions/15752659/thread-pooling-in-c11

class TrainingTask;

//----------------------------------------------------------------------------------------------------------
/// class TrainingSupervision
//----------------------------------------------------------------------------------------------------------
class TrainingSupervision
{
// types
private:
	using task_ptr = std::shared_ptr<TrainingTask>;
// data
private:
	std::atomic_bool			run_{false};
	// task queue
	std::mutex					task_queue_mutex_;
	std::condition_variable		task_queue_cv_;
	std::deque<task_ptr>		task_queue_;
	//~task queue
	std::vector<std::thread>	thread_pool_;

// methods
private:
	void _run_thread();
	void _add_task(task_ptr task);

public:
	~TrainingSupervision();

	void start();
	void stop();
};


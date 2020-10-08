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
	: private util::Logged
{
// types
private:
	using task_ptr = shared_ptr<TrainingTask>;
// data
private:
	atomic_bool			run_{false};
	// task queue
	mutex				task_queue_mutex_;
	condition_variable	task_queue_cv_;
	deque<task_ptr>		task_queue_;
	//~task queue
	vector<thread>		thread_pool_;

// methods
private:
	void _run_thread();
	void _add_task(task_ptr task);

public:
	~TrainingSupervision();

	/// is to be called before start
	void init_tasks(shared_ptr<TrainingTask> initial_task);

	void start();
	void stop();
};


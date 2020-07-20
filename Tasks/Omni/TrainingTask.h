#pragma once
#include <future>
#include <atomic>

//----------------------------------------------------------------------------------------------------------
/// class TrainingTask
//----------------------------------------------------------------------------------------------------------
class TrainingTask
{
// data
private:

protected:
	//std::atomic_bool cancel_{false};

// methods
private:

public:
	TrainingTask();
	virtual ~TrainingTask();

	virtual void run() = 0;
	virtual void cancel() = 0;
	virtual std::shared_ptr<TrainingTask> create_next() = 0;
};
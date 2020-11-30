#pragma once
#include <future>
#include <atomic>

//----------------------------------------------------------------------------------------------------------
/// class TrainingTask
//----------------------------------------------------------------------------------------------------------
class TrainingTask
{
public:
	virtual ~TrainingTask(){}

	virtual void run() = 0;
	virtual std::shared_ptr<TrainingTask> create_next() = 0;
};
#pragma once

class TrainLogger
{
public:
	virtual ~TrainLogger(){}

	virtual TrainLogger* next() = 0;
	//----------------------------------------------------------------------------------------------------------
	/// Training callbacks
	virtual void begin_teach(string label) = 0;
	virtual void set_best(double cur_min_err) = 0;
	virtual void set_last(double err) = 0;
	virtual void set_epoch(unsigned long long num) = 0;
	virtual void end_teach() = 0;
	///~Training callbacks
	//----------------------------------------------------------------------------------------------------------
};
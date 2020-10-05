#pragma once
#include <vector>
#include <atomic>
#include <Shared/Utility/types.hpp>
#include <TrainingTask.h>

using namespace std;

namespace training_task
{
	class MfnProbeContext;

	//----------------------------------------------------------------------------------------------------------
	/// class MfnProbe
	//----------------------------------------------------------------------------------------------------------
	class MfnProbe
		: public TrainingTask
		, private util::Logged
	{
	// types
	// data
	private:
		shared_ptr<MfnProbeContext>	ctx_;
		vector<ptrdiff_t>			columns_;
		ptrdiff_t					target_idx_{-1};
		atomic_bool					stop_flag_{false};

	// methods
	private:
		vector<string> _input_names() const;
		pair<DataFrame, DataView> _prepare_data(const std::string& schema, const std::string& table,
			const std::string& target_name, const std::vector<std::string>& input_names) const;
	public:
		MfnProbe();
		//----------------------------------------------------------------------------------------------------------
		/// TrainingTask impl
		void run() override;
		void cancel() override;
		shared_ptr<TrainingTask> create_next() override;
		///~TrainingTask impl
		//----------------------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------------------
		/// Training callbacks
		bool stop_requested() const noexcept { return stop_flag_; }
		void begin_teach();
		void set_best(double cur_min_err);
		void set_last(double err);
		void set_epoch(unsigned long long num);
		void end_teach();
		void begin_test();
		void add_sample_result(double omega, double target);
		void end_test(unsigned long long true_count, unsigned long long false_count);
		///~Training callbacks
		//----------------------------------------------------------------------------------------------------------
	};
}

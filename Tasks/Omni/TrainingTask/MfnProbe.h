#pragma once
#include <vector>
#include <atomic>
#include <TrainingTask.h>
#include <TrainLogger.h>

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
		shared_ptr<DbAccess>		db_;
		shared_ptr<MfnProbeContext>	ctx_;
		vector<ptrdiff_t>			input_col_idxs_;
		ptrdiff_t					target_idx_{-1};
		atomic_bool&				run_flag_;
		atomic_bool					done_{false};
		TrainLogger*				logger_{nullptr};

	// methods
	private:
		vector<string> _input_names() const;
		string _net_label() const;
		pair<DataFrame, DataView> _prepare_data(const std::string& schema, const std::string& table,
			const std::string& target_name, const std::vector<std::string>& input_names);
		void _run();
	public:
		MfnProbe(shared_ptr<MfnProbeContext> ctx, atomic_bool& run_flag, TrainLogger* logger);
		DbAccess& db() noexcept { return *db_; }
		//----------------------------------------------------------------------------------------------------------
		/// TrainingTask impl
		void run() override;
		shared_ptr<TrainingTask> create_next() override;
		///~TrainingTask impl
		//----------------------------------------------------------------------------------------------------------
		//----------------------------------------------------------------------------------------------------------
		/// Training callbacks
		bool stop_requested() const noexcept;
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

#include "stdafx.h"
#include <Shared/DbAccess/ColumnMetaData.h>
#include <Shared/DbAccess/DbAccess.h>
#include <Shared/Data/Preparation.hpp>
#include <Shared/Neuro/mfn.hpp>
#include <Shared/Neuro/rprop.hpp>
#include <TrainingTask/MfnProbeContext.h>
#include "MfnProbe.h"

using Network = wtom::ml::neuro::net::MultilayerFeedforward;
using Teacher = wtom::ml::neuro::learn::RProp<Network>;

//----------------------------------------------------------------------------------------------------------
training_task::MfnProbe::MfnProbe(shared_ptr<MfnProbeContext> ctx, atomic_bool& run_flag, TrainLogger* logger)
	: ctx_{ctx}
	, db_(std::make_shared<DbAccess>())
	, input_col_idxs_{ctx->next_input_col_idxs()}
	, target_idx_{ctx->target_idx()}
	, run_flag_{run_flag}
	, logger_{logger}
{
	assert(logger_);
}

//----------------------------------------------------------------------------------------------------------
vector<string> training_task::MfnProbe::_input_names() const
{
	const vector<ColumnMetaData>& col_infos{ctx_->col_infos()};

	vector<string> input_names;
	input_names.reserve(input_col_idxs_.size());
	for (ptrdiff_t idx : input_col_idxs_)
	{
		input_names.emplace_back(col_infos[idx].column_);
	}
	return input_names;
}
//----------------------------------------------------------------------------------------------------------
string training_task::MfnProbe::_net_label() const
{
	string label;
	for (ptrdiff_t col : input_col_idxs_)
	{
		label += to_string(col);
		label += ".";
	}
	return label;
}
//----------------------------------------------------------------------------------------------------------
/// 1. Load data frame from DB
/// 2. Fill all non-target-source lacunes by fill_forward policy
/// 3. Drop all rows where target source is NaN
std::pair<DataFrame, DataView> training_task::MfnProbe::_prepare_data(const string& schema, const string& table, const string& target_name,
	const vector<string>& input_names)
{
	constexpr size_t max_tolerated_gap{std::numeric_limits<size_t>::max()};
	// 1.
	vector<string> load_names{input_names};
	load_names.emplace_back(target_name);
	DataFrame df{db().load_data(schema, table, load_names)};
	DataView dv{df};
	// 2.
	for (const string& col_name : load_names)
	{
		if (col_name != target_name)
		{
			wtom::ml::data::fill_missing_forward(df.series(col_name), max_tolerated_gap);
		}
	}
	// 3.
	dv.delete_rows_with_nan();
	return {df, dv};
}

//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::_run()
{
	assert(input_col_idxs_.size() > 0);
	assert(target_idx_ >= 0);

	const vector<ColumnMetaData>& col_infos{ctx_->col_infos()};
	const ColumnMetaData& target_col{col_infos[target_idx_]};
	if (!target_col.normalized_)
	{
		SPDLOG_LOGGER_WARN(log(), "Learning failed. '{:}' should be normalized", target_col.column_);
		return;
	}
	if (target_col.target_error_ <= 0.)
	{
		SPDLOG_LOGGER_WARN(log(), "Learning failed. target error should be positive. current value: '{:}'", target_col.target_error_);
		return;
	}

	const vector<string> input_names{_input_names()};
	pair<DataFrame, DataView> dfv{_prepare_data(db().dest_schema(), target_col.table_, target_col.column_, input_names)};
	const vector<string> target_names{target_col.column_};

	wtom::ml::neuro::net::Config mfn_cfg{{input_col_idxs_.size(), 2, 1}};	// [input:*] > [internal:2] > [output:1]
	Network mfn{mfn_cfg};
	Teacher teacher{move(dfv), input_names, target_names};

	const double best_error = teacher.teach(mfn_cfg, mfn, target_col.target_error_, *this);
	if (best_error <= target_col.target_error_)
	{
		//json j{json::object());
		json j;
		j["mfn"] = mfn;
		db().store_net(_net_label(), best_error, j);
	}
	done_ = true;
}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::run()
{
	assert(run_flag_);
	try
	{
		_run();
	}
	catch (const exception& ex)
	{
		SPDLOG_LOGGER_ERROR(log(), "Learning failed. {:}", ex.what());
	}
	done_ = true;
}
//----------------------------------------------------------------------------------------------------------
shared_ptr<TrainingTask> training_task::MfnProbe::create_next()
{
	assert(run_flag_);
	return make_shared<MfnProbe>(ctx_, run_flag_, done_ ? logger_ : logger_->next());
}
///~TrainingTask impl
//----------------------------------------------------------------------------------------------------------
bool training_task::MfnProbe::stop_requested() const noexcept
{
	return !run_flag_;
}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::begin_teach()
{
	logger_->begin_teach(_net_label());
}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::set_best(double cur_min_err)
{
	logger_->set_best(cur_min_err);
}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::set_last(double err)
{
	logger_->set_last(err);
}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::set_epoch(unsigned long long num)
{
	logger_->set_epoch(num);
}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::end_teach()
{
	logger_->end_teach();
}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::begin_test()
{}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::add_sample_result(double /*omega*/, double /*target*/)
{}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::end_test(unsigned long long /*true_count*/, unsigned long long /*false_count*/)
{}

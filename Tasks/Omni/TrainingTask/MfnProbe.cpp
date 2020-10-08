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
training_task::MfnProbe::MfnProbe(shared_ptr<MfnProbeContext> ctx)
	: ctx_{ctx}
	, input_col_idxs_{ctx->next_input_col_idxs()}
	, target_idx_{ctx->target_idx()}
{}

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
/// 1. Load data frame from DB
/// 2. Fill all non-target-source lacunes by fill_forward policy
/// 3. Drop all rows where target source is NaN
std::pair<DataFrame, DataView> training_task::MfnProbe::_prepare_data(const string& schema, const string& table, const string& target_name,
	const vector<string>& input_names) const
{
	constexpr size_t max_tolerated_gap{std::numeric_limits<size_t>::max()};
	// 1.
	vector<string> load_names{input_names};
	load_names.emplace_back(target_name);
	DataFrame df{ctx_->db().load_data(schema, table, load_names)};
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
void training_task::MfnProbe::run()
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
	pair<DataFrame, DataView> dfv{_prepare_data(ctx_->db().dest_schema(), target_col.table_, target_col.column_, input_names)};
	const vector<string> target_names{target_col.column_};

	//wtom::ml::neuro::net::Config mfn_cfg{ {cols_wt_target, cols_wt_target, cols_wt_target, 1} };
	wtom::ml::neuro::net::Config mfn_cfg{{input_col_idxs_.size(), 2, 1}};
	Network mfn{mfn_cfg};
	Teacher teacher{move(dfv), input_names, target_names};

	teacher.teach(mfn_cfg, mfn, target_col.target_error_, *this);
	//teacher.show_test(mfn, *this);
}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::cancel()
{
	stop_flag_ = true;
}
//----------------------------------------------------------------------------------------------------------
shared_ptr<TrainingTask> training_task::MfnProbe::create_next()
{
	return make_shared<MfnProbe>(ctx_);
}
///~TrainingTask impl
//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::begin_teach()
{}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::set_best(double /*cur_min_err*/)
{}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::set_last(double /*err*/)
{}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::set_epoch(unsigned long long /*num*/)
{}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::end_teach()
{}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::begin_test()
{}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::add_sample_result(double /*omega*/, double /*target*/)
{}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbe::end_test(unsigned long long /*true_count*/, unsigned long long /*false_count*/)
{}

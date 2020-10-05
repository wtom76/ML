#include "stdafx.h"
#include <algorithm>
#include "MfnProbeContext.h"
#include <Shared/DbAccess/ColumnMetaData.h>
#include <Shared/DbAccess/DbAccess.h>

//----------------------------------------------------------------------------------------------------------
training_task::MfnProbeContext::MfnProbeContext(shared_ptr<DbAccess> db)
	: db_{move(db)}
	, col_infos_{db_->load_meta_data()}
{
	_init();
}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbeContext::_init()
{
	assert(full_input_col_idxs_.empty());
	assert(next_permutation_.empty());

	if (col_infos_.empty())
	{
		throw runtime_error("MfnProbeContext error: metadata is empty. Should be more then 1.");
	}
	if (col_infos_.size() == 1)
	{
		throw runtime_error("MfnProbeContext error: metadata contains 1 column. Should be more then 1.");
	}

	full_input_col_idxs_.reserve(col_infos_.size() - 1);
	next_permutation_.reserve(col_infos_.size() - 1);

	bool found{false};
	const ptrdiff_t count{static_cast<ptrdiff_t>(col_infos_.size())};
	for (ptrdiff_t i = 0; i != count; ++i)
	{
		if (col_infos_[i].is_target_)
		{
			if (found)
			{
				throw runtime_error("MfnProbeContext error: more than one target is found. Should be one.");
			}
			target_idx_ = i;
			found = true;
		}
		else
		{
			full_input_col_idxs_.emplace_back(i);
		}
	}
	if (!found)
	{
		throw runtime_error("MfnProbeContext error: no target is found. Should be one.");
	}

	next_permutation_.emplace_back(0);
}
//----------------------------------------------------------------------------------------------------------
// 1. increment last number
// 2. if can't, reset it and increment prev number
// ...
// 3. if can't increment anything, 
void training_task::MfnProbeContext::_make_next_permutation()
{

}
//----------------------------------------------------------------------------------------------------------
vector<ptrdiff_t> training_task::MfnProbeContext::next_input_col_idxs()
{
	vector<ptrdiff_t> result;
	result.reserve(next_permutation_.size());
	for (ptrdiff_t idx_of_idx : next_permutation_)
	{
		result.emplace_back(full_input_col_idxs_[idx_of_idx]);
	}
	_make_next_permutation();
}

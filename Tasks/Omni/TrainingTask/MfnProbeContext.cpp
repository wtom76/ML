#include "stdafx.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include "MfnProbeContext.h"
#include <Shared/DbAccess/ColumnMetaData.h>
#include <Shared/DbAccess/DbAccess.h>

//----------------------------------------------------------------------------------------------------------
training_task::MfnProbeContext::MfnProbeContext(shared_ptr<DbAccess> db)
	: db_{move(db)}
	, col_infos_{db_->load_meta_data()}
{
	srand(time(nullptr));
	_init();
}
//----------------------------------------------------------------------------------------------------------
void training_task::MfnProbeContext::_init()
{
	if (col_infos_.empty())
	{
		throw runtime_error("MfnProbeContext error: metadata is empty. Should be more then 1.");
	}
	if (col_infos_.size() == 1)
	{
		throw runtime_error("MfnProbeContext error: metadata contains 1 column. Should be more then 1.");
	}

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
	}
	if (!found)
	{
		throw runtime_error("MfnProbeContext error: no target is found. Should be one.");
	}
	// temp HACK
	if (col_infos_[target_idx_].target_error_ <= 0.)
	{
		col_infos_[target_idx_].target_error_ = 0.01;
	}
}
//----------------------------------------------------------------------------------------------------------
vector<ptrdiff_t> training_task::MfnProbeContext::next_input_col_idxs()
{
	vector<ptrdiff_t> result;
	const ptrdiff_t count{static_cast<ptrdiff_t>(col_infos_.size() - 1)};
	if (count <= 1)
	{
		assert(false);
		return result;
	}

	result.reserve(count);
	while (result.empty())
	{
		for (ptrdiff_t i = 0; i != count; ++i)
		{
			if (!col_infos_[i].is_target_ && rand() % 2)
			{
				result.emplace_back(i);
			}
		}
	}
	return result;
}

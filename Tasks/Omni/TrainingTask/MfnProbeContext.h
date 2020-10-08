#pragma once
#include <vector>
#include <string>
#include <memory>
#include <Shared/DbAccess/ColumnMetaData.h>
#include <Shared/DbAccess/DbAccess.h>

struct ColumnMetaData;

namespace training_task
{
	//----------------------------------------------------------------------------------------------------------
	/// class MfnProbeContext
	//----------------------------------------------------------------------------------------------------------
	class MfnProbeContext
	{
	// types
	// data
	private:
		shared_ptr<DbAccess>	db_;
		vector<ColumnMetaData>	col_infos_;
		ptrdiff_t				target_idx_{-1};			// in col_infos_

	// methods
	private:
		void _init();
	public:
		explicit MfnProbeContext(shared_ptr<DbAccess> db);

		const vector<ColumnMetaData>& col_infos() const noexcept { return col_infos_; }
		ptrdiff_t target_idx() const noexcept { return target_idx_; }
		/// \return column indexes of inputs
		vector<ptrdiff_t> next_input_col_idxs();

		DbAccess& db() noexcept { return *db_; }
	};
}
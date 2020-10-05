#pragma once
#include <vector>
#include <string>
#include <memory>

struct ColumnMetaData;
class DbAccess;

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
		vector<ptrdiff_t>		full_input_col_idxs_;		// in col_infos_
		vector<ptrdiff_t>		next_permutation_;			// of full_input_col_idxs_

	// methods
	private:
		void _init();
		void _make_next_permutation();
	public:
		explicit MfnProbeContext(shared_ptr<DbAccess> db);

		const vector<ColumnMetaData>& col_infos() const noexcept { return col_infos_; }
		ptrdiff_t target_idx() const noexcept { return target_idx_; }
		/// \returns current sequence and make next
		vector<ptrdiff_t> next_input_col_idxs();

		DbAccess& db() noexcept { return *db_; }
	};
}
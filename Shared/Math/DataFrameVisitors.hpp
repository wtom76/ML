#include "stdafx.h"
#include <limits>

using namespace std;

//----------------------------------------------------------------------------------------------------------
/// MinMaxValidCounter counts non-nan column entries
//----------------------------------------------------------------------------------------------------------
template<typename value_type, typename index_type = unsigned long>
struct MinMaxValidCounter
{
	void operator() (const index_type&, const value_type& val) noexcept
	{
		if (hmdf::is_nan__(val))
		{
			return;
		}
		++valid_count_;
		if (val > max_)
		{
			max_ = val;
		}
		if (val < min_)
		{
			min_ = val;
		}
	}
	void pre() noexcept {}
	void post() noexcept {}
	std::size_t valid_count() const noexcept { return valid_count_; }
	value_type min() const noexcept { return min_; }
	value_type max() const noexcept { return max_; }

	MinMaxValidCounter() = default;

private:
	std::size_t	valid_count_{};
	value_type  min_{std::numeric_limits<value_type>::max()};
	value_type  max_{-std::numeric_limits<value_type>::max()};
};

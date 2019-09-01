#pragma once
#include <vector>

namespace wtom::ml::Data
{
	//-----------------------------------------------------------------------------------------------------
	// class Index
	//-----------------------------------------------------------------------------------------------------
	class Index
	{
	// types
	// data
	private:
		std::vector<__int64> index_;

	// methods
	public:
		__int64 resolve(__int64 index) const { return index_[index]; }
	};
}
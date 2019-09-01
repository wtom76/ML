#pragma once
#include <memory>
#include <Shared/Data/Index.hpp>
#include <Shared/Data/Frame.hpp>

namespace wtom::ml::Data
{
	//-----------------------------------------------------------------------------------------------------
	// class View
	//-----------------------------------------------------------------------------------------------------
	template <typename value_t>
	class View
	{
	// types
	public:
		using Series = std::vector<value_t>;
		using Data = std::vector<Series>;

	// data
	private:
		std::shared_ptr<Data>	data_;
		Index					row_index_;

	// methods
	public:
	};
}
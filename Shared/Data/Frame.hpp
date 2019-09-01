#pragma once
#include <memory>
#include <stdexcept>

namespace wtom::ml::Data
{
	//-----------------------------------------------------------------------------------------------------
	// class Frame
	//-----------------------------------------------------------------------------------------------------
	template <typename value_t>
	class Frame
	{
	// types
	public:
		using series_t = std::vector<value_t>;
		using data_t = std::vector<series_t>;
		using index_t = __int64;

	// data
	private:
		data_t columns_;

	// methods
	public:
		void	setColumns(__int64 columns);
		void	reserve(__int64 rows) noexcept;
		void	clear() noexcept;

		index_t	push_back(const std::vector<value_t>& row);
	};

	//-----------------------------------------------------------------------------------------------------
	template <typename value_t>
	inline void Frame<value_t>::setColumns(__int64 columns)
	{
		columns_.resize(columns);
	}
	//-----------------------------------------------------------------------------------------------------
	template <typename value_t>
	inline void Frame<value_t>::reserve(__int64 rows) noexcept
	{
		for (auto& col : columns_)
		{
			col.reserve(rows);
		}
	}
	//-----------------------------------------------------------------------------------------------------
	template <typename value_t>
	inline void Frame<value_t>::clear() noexcept
	{
		columns_.clear();
	}
	//-----------------------------------------------------------------------------------------------------
	template <typename value_t>
	inline typename Frame<value_t>::index_t Frame<value_t>::push_back(const std::vector<value_t>& row)
	{
		if (columns_.empty())
		{
			return -1;
		}

		auto src_field_i = row.cbegin();
		auto src_field_e = row.cend();

		for (auto& col : columns_)
		{
			if (src_field_i == src_field_e)
			{
				throw std::out_of_range("too long row is pushed to Frame");
			}
			col.push_back(*src_field_i++);
		}

		return columns_[0].size() - 1;
	}
}
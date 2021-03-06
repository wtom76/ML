#pragma once
#include <numeric>
#include <Shared/Data/Frame.hpp>

namespace wtom::ml::data
{
	//-----------------------------------------------------------------------------------------------------
	// class View
	//-----------------------------------------------------------------------------------------------------
	template <typename index_t, typename value_t>
	class View
	{
	// types
	public:

	// data
	private:
		Frame<index_t, value_t>&	frame_;
		std::vector<size_t>			index_;

	// methods
	public:
		explicit View(Frame<index_t, value_t>& frame);

		size_t row_count() const noexcept { return index_.size(); }
		const Frame<index_t, value_t>& frame() const noexcept { return frame_; }
		size_t frame_index(size_t view_index) const noexcept;
		bool row_has_nan(size_t row_idx) const noexcept;
		void delete_rows_with_nan() noexcept;
	};
	//-----------------------------------------------------------------------------------------------------
	template <typename index_t, typename value_t>
	inline View<index_t, value_t>::View(Frame<index_t, value_t>& frame)
		: frame_{frame}
	{
		index_.resize(frame.index().size());
		std::iota(index_.begin(), index_.end(), 0);
	}
	//-----------------------------------------------------------------------------------------------------
	template <typename index_t, typename value_t>
	inline size_t View<index_t, value_t>::frame_index(size_t view_index) const noexcept
	{
		return index_[view_index];
	}
	//-----------------------------------------------------------------------------------------------------
	template <typename index_t, typename value_t>
	inline bool View<index_t, value_t>::row_has_nan(size_t row_idx) const noexcept
	{
		for (const auto& series : frame_.data())
		{
			if (std::isnan(series[index_[row_idx]]))
			{
				return true;
			}
		}
		return false;
	}
	//-----------------------------------------------------------------------------------------------------
	template <typename index_t, typename value_t>
	inline void View<index_t, value_t>::delete_rows_with_nan() noexcept
	{
		std::vector<size_t> new_index;
		new_index.reserve(index_.size());
		for (size_t i = 0; i != index_.size(); ++i)
		{
			if (!row_has_nan(i))
			{
				new_index.emplace_back(index_[i]);
			}
		}
		index_ = move(new_index);
	}
}
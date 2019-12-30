#pragma once
#include <memory>
#include <limits>

namespace wtom::ml::data
{
	//-----------------------------------------------------------------------------------------------------
	// class Frame
	//-----------------------------------------------------------------------------------------------------
	template <typename index_t, typename value_t>
	class Frame
	{
	// types
	public:
		using index_series_t = std::vector<index_t>;
		using series_t = std::vector<value_t>;
		using data_t = std::vector<series_t>;
		using names_t = std::vector<std::string>;

	// data
	private:
		index_series_t	index_series_;
		data_t			data_;
		names_t			series_names_;

	// methods
	public:
		size_t row_count() const noexcept { return index_series_.size();  }
		size_t col_count() const noexcept { return data_.size(); }
		index_series_t& index() noexcept { return index_series_; }
		const index_series_t& index() const noexcept { return index_series_; }
		data_t& data() noexcept { return data_; }
		const data_t& data() const noexcept { return data_; }
		series_t* series(const std::string& name) noexcept;
		const series_t* series(const std::string& name) const noexcept;
		void reset(index_series_t&& index_series, const names_t& series_names, value_t initial_value);
		series_t* create_series(const std::string& name, value_t initial_value);
	};
	//-----------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------
	template <typename index_t, typename value_t>
	inline typename Frame<index_t, value_t>::series_t* Frame<index_t, value_t>::series(const std::string& name) noexcept
	{
		const auto name_i = std::find(series_names_.cbegin(), series_names_.cend(), name);
		assert(name_i != series_names_.cend());
		return name_i == series_names_.cend() ? nullptr : &data_[std::distance(series_names_.cbegin(), name_i)];
	}
	//-----------------------------------------------------------------------------------------------------
	template <typename index_t, typename value_t>
	inline const typename Frame<index_t, value_t>::series_t* Frame<index_t, value_t>::series(const std::string& name) const noexcept
	{
		return const_cast<Frame<index_t, value_t>*>(this)->series(name);
	}
	//-----------------------------------------------------------------------------------------------------
	template <typename index_t, typename value_t>
	inline void Frame<index_t, value_t>::reset(index_series_t&& index_series, const names_t& series_names, value_t initial_value)
	{
		series_names_ = series_names;
		index_series_ = move(index_series);
		data_.resize(series_names_.size());
		for (series_t& col : data_)
		{
			col.resize(index_series_.size(), initial_value);
		}
	}
	//-----------------------------------------------------------------------------------------------------
	template <typename index_t, typename value_t>
	inline typename Frame<index_t, value_t>::series_t* Frame<index_t, value_t>::create_series(const std::string& name, value_t initial_value)
	{
		const auto name_i = std::find(series_names_.cbegin(), series_names_.cend(), name);
		if (name_i != series_names_.cend())
		{
			assert(false);
			return nullptr;
		}
		data_.emplace_back(index_series_.size(), initial_value);
		series_names_.emplace_back(name);
		return &data_.back();
	}
}
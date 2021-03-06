#pragma once
#include <string>
#include <memory>
#include <limits>
#include <algorithm>
#include <cassert>

namespace wtom::ml::data
{
	using namespace std::literals::string_literals;

	//-----------------------------------------------------------------------------------------------------
	/// SeriesAttributes
	//-----------------------------------------------------------------------------------------------------
	enum class SeriesAttributes : int
	{
		Null	= 0x0000,
		Price	= 0x0001,
		Qty		= 0x0002
	};

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

		series_t& series(const std::string& name);
		const series_t& series(const std::string& name) const;

		series_t& series(ptrdiff_t idx);
		const series_t& series(ptrdiff_t idx) const;

		const names_t& names() const noexcept { return series_names_; }

		void reset(index_series_t&& index_series, const names_t& series_names, value_t initial_value);
		
		series_t* create_series(const std::string& name, value_t initial_value);

		/// appends other to this if indexes are equal, throws runtime_error otherwise
		template <class FrameT>
		void append_equal(FrameT&& other);
	};
	//-----------------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------------
	template <typename index_t, typename value_t>
	inline typename Frame<index_t, value_t>::series_t& Frame<index_t, value_t>::series(const std::string& name)
	{
		const auto name_i = std::find(series_names_.cbegin(), series_names_.cend(), name);
		if (name_i == series_names_.cend())
		{
			throw std::runtime_error("No '"s + name + "' series"s);
		}
		return data_[std::distance(series_names_.cbegin(), name_i)];
	}
	//-----------------------------------------------------------------------------------------------------
	template <typename index_t, typename value_t>
	inline const typename Frame<index_t, value_t>::series_t& Frame<index_t, value_t>::series(const std::string& name) const
	{
		return const_cast<Frame<index_t, value_t>*>(this)->series(name);
	}
	//-----------------------------------------------------------------------------------------------------
	template <typename index_t, typename value_t>
	inline typename Frame<index_t, value_t>::series_t& Frame<index_t, value_t>::series(ptrdiff_t idx)
	{
		return data_[idx];
	}
	//-----------------------------------------------------------------------------------------------------
	template <typename index_t, typename value_t>
	inline const typename Frame<index_t, value_t>::series_t& Frame<index_t, value_t>::series(ptrdiff_t idx) const
	{
		return const_cast<Frame<index_t, value_t>*>(this)->series(idx);
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
	//-----------------------------------------------------------------------------------------------------
	template <typename index_t, typename value_t>
	template <class FrameT>
	inline void Frame<index_t, value_t>::append_equal(FrameT&& other)
	{
		if (!std::equal(index_series_.cbegin(), index_series_.cend(),
			other.index_series_.cbegin(), other.index_series_.cend()))
		{
			throw std::runtime_error("indexes of frames are not equal"s);
		}
		data_.reserve(data_.size() + other.data_.size());
		series_names_.reserve(series_names_.size() + other.series_names_.size());
		auto data_i = other.data_.begin();
		const auto data_e = other.data_.cend();
		auto name_i = other.series_names_.begin();
		for (; data_i != data_e; ++data_i, ++name_i)
		{
			data_.emplace_back(std::forward<series_t>(*data_i));
			series_names_.emplace_back(std::forward<std::string>(*name_i));
		}
	}
}
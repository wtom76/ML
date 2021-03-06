#pragma once
#include <limits>
#include <chrono>
#include <Shared/Data/Frame.hpp>
#include <Shared/Data/View.hpp>

//-------------------------------------------------------------------------------------------------
template <typename EnumT>
constexpr typename std::underlying_type<EnumT>::type to_numeric(EnumT val) noexcept
{
	return static_cast<typename std::underlying_type<EnumT>::type>(val);
}

//-------------------------------------------------------------------------------------------------
using DataFrame = wtom::ml::data::Frame<std::chrono::system_clock::time_point, double>;
using DataView = wtom::ml::data::View<std::chrono::system_clock::time_point, double>;

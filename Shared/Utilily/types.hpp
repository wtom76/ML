#pragma once

//-------------------------------------------------------------------------------------------------
template <typename EnumT>
constexpr typename std::underlying_type<EnumT>::type to_numeric(EnumT val) noexcept
{
	return static_cast<typename std::underlying_type<EnumT>::type>(val);
}

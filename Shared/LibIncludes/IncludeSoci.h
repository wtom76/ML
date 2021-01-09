#pragma once

/// Requires ObjectDomain.hpp to be included beforehand

#define SOCI_DLL

#include <soci/soci.h>
#include <soci/postgresql/soci-postgresql.h>
#if defined (SOCI_INCLUDE_ODBC)
	#pragma warning(push)
	#pragma warning(disable: 4996)
	#include <soci/odbc/soci-odbc.h>
	#pragma warning(pop)
#endif
#include <string>
#include <chrono>
#include <optional>

namespace soci
{
	//------------------------------------------------------------------------------------------------------
	/// system_clock::time_point support
	template <>
	struct type_conversion<std::chrono::system_clock::time_point>
	{
		using base_type = std::tm;

		static void from_base(base_type base_val, indicator ind, std::chrono::system_clock::time_point& val)
		{
			switch (ind)
			{
			case i_null:
				throw soci_error("Null datetime is not supported");
				break;
			default:
				val = std::chrono::system_clock::from_time_t(_mkgmtime64(&base_val));
			}
		}

		static void to_base(std::chrono::system_clock::time_point val, base_type& base_val, indicator& ind)
		{
			__time64_t tt{};
			const errno_t err = _gmtime64_s(&base_val, &(tt = std::chrono::system_clock::to_time_t(val)));
			if (err)
			{
				throw soci_error("soci::type_conversion: failed to convert system_clock::time_point to tm");
			}
			ind = i_ok;
		}
	};
	//------------------------------------------------------------------------------------------------------
	/// optional system_clock::time_point support
	template <>
	struct type_conversion<std::optional<std::chrono::system_clock::time_point>>
	{
		using base_type = std::tm;

		static void from_base(base_type base_val, indicator ind, std::optional<std::chrono::system_clock::time_point>& val)
		{
			switch (ind)
			{
			case i_null:
				val = {};
				break;
			default:
				val = std::chrono::system_clock::from_time_t(_mkgmtime64(&base_val));
			}
		}

		static void to_base(std::optional<std::chrono::system_clock::time_point> val, base_type& base_val, indicator& ind)
		{
			if (val)
			{
				__time64_t tt{};
				const errno_t err = _gmtime64_s(&base_val, &(tt = std::chrono::system_clock::to_time_t(val.value())));
				if (err)
				{
					throw soci_error("soci::type_conversion: failed to convert system_clock::time_point to tm");
				}
				ind = i_ok;
			}
			else
			{
				ind = i_null;
			}
		}
	};
	//------------------------------------------------------------------------------------------------------
	/// bool support
	template <>
	struct type_conversion<bool>
	{
		using base_type = int;

		static void from_base(base_type base_val, indicator ind, bool& val)
		{
			switch (ind)
			{
			case i_null:
				throw soci_error("Null boolean is not supported");
			default:
				val = base_val;
			}
		}
		static void to_base(bool val, base_type& base_val, indicator& ind)
		{
			base_val = static_cast<base_type>(val);
			ind = i_ok;
		}
	};
	//------------------------------------------------------------------------------------------------------
	/// double support
	template <>
	struct type_conversion<double>
	{
		using base_type = double;

		static void from_base(base_type base_val, indicator ind, double& val)
		{
			switch (ind)
			{
			case i_null:
				val = std::numeric_limits<double>::quiet_NaN();
				break;
			default:
				val = base_val;
			}
		}
		static void to_base(double val, base_type& base_val, indicator& ind)
		{
			if (isnan<double>(val))
			{
				ind = i_null;
				base_val = std::numeric_limits<double>::quiet_NaN();
			}
			else
			{
				base_val = static_cast<base_type>(val);
				ind = i_ok;
			}
		}
	};
}
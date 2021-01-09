#include "stdafx.h"
#include <chrono>
#include <Shared/LibIncludes/IncludeSoci.h>
#include "DbAccess.h"

using namespace soci;

//----------------------------------------------------------------------------------------------------------
// Impl
//----------------------------------------------------------------------------------------------------------
class DbAccess::Impl
{
public:
	session sql_;

public:
	Impl(const std::string& connection_str);
};
//----------------------------------------------------------------------------------------------------------
DbAccess::Impl::Impl(const std::string& connection_str)
	: sql_{connection_str}
{
}
//----------------------------------------------------------------------------------------------------------
DbAccess::DbAccess(const std::string& connection_str)
	: impl_{make_unique<Impl>(connection_str)}
{
	SPDLOG_LOGGER_DEBUG(log(), "DbAccess is connected to {:s}", connection_str);
}
//----------------------------------------------------------------------------------------------------------
DbAccess::~DbAccess()
{
}
//----------------------------------------------------------------------------------------------------------
set<string> DbAccess::tableColumns(const string& schema_name, const string& table_name) const
{
	try
	{
		rowset<string> rs = (impl_->sql_.prepare <<
			"SELECT column_name FROM information_schema.columns "
			"WHERE table_schema = :schema_name AND table_name = :table_name",
			use(schema_name),
			use(table_name));
		SPDLOG_LOGGER_TRACE(log(), "column_name from information_schema.columns is loaded");
		return set<string>{begin(rs), end(rs)};
	}
	catch (const exception& ex)
	{
		throw runtime_error("Failed to load columns. "s + ex.what());
	}
}
//----------------------------------------------------------------------------------------------------------
/// 1. Add target column to dest table
/// 2. Update metadata
void DbAccess::add_column(const string& dest_schema_name, const ColumnMetaData& col_meta)
{
	try
	{
		const ColumnPath origin = col_meta.column_path();
		 // 1.
		impl_->sql_ <<
			"ALTER TABLE " << dest_schema_name << "." << col_meta.table_ <<
			" ADD COLUMN " << col_meta.column_ << " " << origin.data_type_;
		// 2.
		const string origin_str = origin.to_string();
		impl_->sql_ <<
			"INSERT INTO " << dest_schema_name << ".meta_data (\"table\", \"column\", \"description\", \"origin\", \"unit_id\", \"is_target\")"
			" VALUES (:dest_table_name, :column, :description, :origin, :unit_id, :is_target);"
			, use(col_meta.table_,		"dest_table_name"s)
			, use(col_meta.description_,"description"s)
			, use(col_meta.column_,		"column"s)
			, use(origin_str,			"origin"s)
			, use(col_meta.unit_id_,	"unit_id"s)
			, use(col_meta.is_target_,	"is_target"s);

		SPDLOG_LOGGER_TRACE(log(), "column is inserted into meta_data");
	}
	catch (const exception & ex)
	{
		throw runtime_error("Failed to load columns. "s + ex.what());
	}
}
//----------------------------------------------------------------------------------------------------------
/// 2. Copy data to dest table
/// 3. Update metadata
void DbAccess::copy_column_data(const string& dest_schema_name, const string& dest_table_name,
	const ColumnPath& col_info)
{
	try
	{
		// 1.
		impl_->sql_ <<
			"INSERT INTO " << dest_schema_name << "." << dest_table_name << " (date, " << col_info.column_ << ")"
			" SELECT date, " << col_info.column_ << " FROM " << col_info.schema_ << "." << col_info.table_ <<
			" ON CONFLICT (date) DO UPDATE SET " << col_info.column_ << " = excluded." << col_info.column_;
	}
	catch (const exception & ex)
	{
		throw runtime_error("Failed to load columns. "s + ex.what());
	}
}
//----------------------------------------------------------------------------------------------------------
void DbAccess::delete_column(const string& dest_schema_name, const ColumnMetaData& col_info)
{
	try
	{
		impl_->sql_ <<
			"ALTER TABLE " << dest_schema_name << "." << col_info.table_ << " DROP COLUMN " << col_info.column_;
		impl_->sql_ <<
			"DELETE FROM " << dest_schema_name << ".meta_data WHERE id = :id;"
		, use(col_info.id_);
	}
	catch (const exception& ex)
	{
		throw runtime_error("Failed to delete column. "s + ex.what());
	}
}
//----------------------------------------------------------------------------------------------------------
vector<UnitInfo> DbAccess::load_units() const
{
	rowset<row> rs{impl_->sql_.prepare << "SELECT id, name FROM ready.units;"};

	vector<UnitInfo> result;
	for (const auto& row : rs)
	{
		result.emplace_back(row.get<long long>(0), row.get<string>(1));
	}

	return result;
}
//----------------------------------------------------------------------------------------------------------
vector<ColumnMetaData> DbAccess::load_meta_data() const
{
	static constexpr int id_idx				= 0;
	static constexpr int table_idx			= 1;
	static constexpr int column_idx			= 2;
	static constexpr int description_idx	= 3;
	static constexpr int origin_idx			= 4;
	static constexpr int normalized_idx		= 5;
	static constexpr int norm_min_idx		= 6;
	static constexpr int norm_max_idx		= 7;
	static constexpr int unit_id_idx		= 8;
	static constexpr int date_min_idx		= 9;
	static constexpr int date_max_idx		= 10;
	static constexpr int is_target_idx		= 11;

	vector<ColumnMetaData> result;

	try
	{
		rowset<row> rs = (impl_->sql_.prepare <<
		"SELECT id, \"table\", \"column\", description, origin, normalized, norm_min, norm_max, unit_id, date_min, date_max, is_target FROM ready.meta_data;");
		for (auto& row : rs)
		{
			ColumnMetaData& data = result.emplace_back(ColumnMetaData{});
			data.id_			= row.get<long long>(id_idx);
			data.table_			= row.get<string>(table_idx);
			data.column_		= row.get<string>(column_idx);
			data.description_	= row.get_indicator(description_idx) == i_ok ? row.get<string>(description_idx) : string{};
			data.origin_		= row.get<string>(origin_idx);
			data.normalized_	= row.get<bool>(normalized_idx);
			data.norm_min_		= row.get<double>(norm_min_idx);
			data.norm_max_		= row.get<double>(norm_max_idx);
			data.unit_id_		= row.get<long long>(unit_id_idx);
			data.date_min_		= row.get<decltype(data.date_min_)>(date_min_idx);
			data.date_max_		= row.get<decltype(data.date_max_)>(date_max_idx);
			data.is_target_		= row.get<bool>(is_target_idx, false);
		}
		SPDLOG_LOGGER_TRACE(log(), "meta_data is loaded");
		return result;
	}
	catch (const exception& ex)
	{
		throw runtime_error("Failed to load meta data. "s + ex.what());
	}
}
//----------------------------------------------------------------------------------------------------------
void DbAccess::store_column(const ColumnMetaData& col_info, const DataFrame& data) const
{
	try
	{
		vector<system_clock::time_point>& idx = const_cast<vector<system_clock::time_point>&>(data.index());
		const vector<double>& values = data.series(col_info.column_);
		vector<indicator> value_flags;
		{
			value_flags.resize(idx.size(), i_ok);
			for (size_t i = 0; i != values.size(); ++i)
			{
				if (std::isnan(values[i]))
				{
					value_flags[i] = i_null;
				}
			}
		}
		impl_->sql_
			<< "UPDATE ready." << col_info.table_ << " SET " << col_info.column_ << " = :val WHERE date = :date"
			, use(values, value_flags, "val"s)
			, use(idx, "date"s);
	}
	catch (const exception & ex)
	{
		throw runtime_error("Failed to store column data. "s + ex.what());
	}

	store_column_metadata(col_info);
}
//----------------------------------------------------------------------------------------------------------
void DbAccess::store_column_metadata(const ColumnMetaData& col_info) const
{
	try
	{
		const double norm_min = col_info.normalized_ ? col_info.norm_min_ : 0.;
		const double norm_max = col_info.normalized_ ? col_info.norm_max_ : 0.;

		impl_->sql_
			<< "UPDATE ready.meta_data "
			"SET normalized = :normalized, norm_min = :norm_min, norm_max = :norm_max, date_min = :date_min, date_max = :date_max, is_target = :is_target "
			"WHERE id = :id"
			, use(col_info.normalized_, "normalized")
			, use(norm_min, "norm_min")
			, use(norm_max, "norm_max")
			, use(col_info.id_, "id")
			, use(col_info.date_min_, "date_min")
			, use(col_info.date_max_, "date_max")
			, use(col_info.is_target_, "is_target");
	}
	catch (const exception & ex)
	{
		throw runtime_error("Failed to store column metadata. "s + ex.what());
	}
}
//----------------------------------------------------------------------------------------------------------
void DbAccess::store_column_metadata_is_target(const ColumnMetaData& col_info) const
{
	try
	{
		impl_->sql_ << "UPDATE ready.meta_data SET is_target = :is_target WHERE id = :id"
			, use(col_info.id_, "id")
			, use(col_info.is_target_, "is_target");
	}
	catch (const exception & ex)
	{
		throw runtime_error("Failed to store column metadata. "s + ex.what());
	}
}
//----------------------------------------------------------------------------------------------------------
// 1. count
// 2. index
// 3. data
DataFrame DbAccess::load_data(const string& schema, const string& table_name, const vector<string>& col_names) const
{
	DataFrame result;

	const auto lock{cs_.read_lock()};

	try
	{
		ptrdiff_t count = 0;
		// 1.
		{
			impl_->sql_ << "SELECT count(date) FROM " << schema << "." << table_name, into(count);
			if (count <= 0)
			{
				return result;
			}
		}
		vector<indicator> ind(count);
		// 2.
		{
			vector<system_clock::time_point> idx(count);
			impl_->sql_ << "SELECT date FROM " << schema << "." << table_name << " ORDER BY date", into(idx);
			result.reset(move(idx), col_names, std::numeric_limits<double>::quiet_NaN());
		}
		// 3.
		for (size_t i = 0; i != col_names.size(); ++i)
		{
			vector<double>& col = result.data()[i];
			impl_->sql_ << "SELECT " << col_names[i] << " FROM " << schema << "." << table_name << " ORDER BY date", into(col, ind);
		}
		SPDLOG_LOGGER_TRACE(log(), "{:s} is loaded", table_name);
		return result;
	}
	catch (const exception& ex)
	{
		throw runtime_error("Failed to load column data. "s + ex.what());
	}
}
//----------------------------------------------------------------------------------------------------------
void DbAccess::store_net(const std::string& descriptor, double error, const json& data)
{
	try
	{
		const string data_str{data.dump()};
		impl_->sql_
			<< "INSERT INTO ready.trained_nets "
			"(descriptor, time_stamp, train_error, data) "
			"VALUES (:descriptor, current_timestamp, :train_error, :data) "
			, use(descriptor, "descriptor")
			, use(error, "train_error")
			, use(data_str, "data");
	}
	catch (const exception & ex)
	{
		throw runtime_error("Failed to store net. "s + ex.what());
	}
}
//----------------------------------------------------------------------------------------------------------
void DbAccess::load_net(const std::string& descriptor, double& error, json& dest)
{
	try
	{
		dest.clear();
		string data;
		impl_->sql_ << "SELECT data, train_error FROM ready.trained_nets WHERE descriptor = :descriptor",
			into(data), into(error), use(descriptor);
		dest = json::parse(data);
	}
	catch (const exception & ex)
	{
		throw runtime_error("Failed to load net. "s + ex.what());
	}
}

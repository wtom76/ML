#include "stdafx.h"
#include <chrono>
#include <soci/soci.h>
#include <soci/postgresql/soci-postgresql.h>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDate>
#include "DbAccess.h"

using namespace std;
using namespace chrono;
using namespace soci;

namespace soci
{
	//------------------------------------------------------------------------------------------------------
	/// system_clock::time_point support
	template <>
	struct type_conversion<hmdf::DateTime>
	{
		using base_type = tm;

		static void from_base(base_type base_val, indicator ind, hmdf::DateTime& val)
		{
			switch (ind)
			{
			case i_null:
				val = hmdf::DateTime{};
				//throw soci_error("Null DateTime is not supported");
			default:
				val = hmdf::DateTime{static_cast<hmdf::DateTime::DateType>(
					(base_val.tm_year + 1900) * 10000 + (base_val.tm_mon + 1) * 10 + base_val.tm_mday)};
			}
		}
		static void to_base(hmdf::DateTime val, base_type& base_val, indicator& ind)
		{
			memset(&base_val, 0, sizeof(base_val));
			base_val.tm_year = val.year() - 1900;
			base_val.tm_mon = val.dmonth() - 1;
			base_val.tm_mday = val.days_in_month();
			ind = i_ok;
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
	/// bool support
	template <>
	struct type_conversion<double>
	{
		using base_type = double;

		static void from_base(base_type base_val, indicator ind, double& val)
		{
			switch (ind)
			{
			case i_null:
				val = numeric_limits<double>::quiet_NaN();
			default:
				val = base_val;
			}
		}
		static void to_base(double val, base_type& base_val, indicator& ind)
		{
			if (isnan<double>(val))
			{
				ind = i_null;
				base_val = numeric_limits<double>::quiet_NaN();
			}
			else
			{
				base_val = static_cast<base_type>(val);
				ind = i_ok;
			}
		}
	};
}

//----------------------------------------------------------------------------------------------------------
// Impl
//----------------------------------------------------------------------------------------------------------
class DbAccess::Impl
{
public:
	session sql_;

public:
	Impl();
};
//----------------------------------------------------------------------------------------------------------
DbAccess::Impl::Impl()
	: sql_{"postgresql://host=localhost dbname=ML user=ml_user password=ml_user"}
{
}
//----------------------------------------------------------------------------------------------------------
DbAccess::DbAccess()
	: QObject(nullptr)
	, impl_{make_unique<Impl>()}
{
	QSqlDatabase db = QSqlDatabase::addDatabase(QString::fromStdString("QPSQL"));
	db.setHostName(QString::fromStdString("localhost"));
	db.setDatabaseName(QString::fromStdString("ML"));
	db.setUserName(QString::fromStdString("ml_user"));
	db.setPassword(QString::fromStdString("ml_user"));
	if (!db.open())
	{
		throw runtime_error("Failed to connect to database. "s +
			db.lastError().text().toStdString());
	}
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
		return set<string>{begin(rs), end(rs)};
	}
	catch (const exception& ex)
	{
		throw runtime_error("Failed to load columns. "s + ex.what());
	}
}
//----------------------------------------------------------------------------------------------------------
/// 1. Add target column to dest table
/// 2. Copy data to dest table
/// 3. Update metadata
void DbAccess::addColumn(const string& dest_schema_name, const string& dest_table_name, int unit_id,
	const ColumnPath& col_info)
{
	try
	{
		 // 1.
		impl_->sql_ <<
			"ALTER TABLE " << dest_schema_name << "." << dest_table_name <<
			" ADD COLUMN " << col_info.column_ << " " << col_info.data_type_;
		// 2.
		impl_->sql_ <<
			"INSERT INTO " << dest_schema_name << "." << dest_table_name << " (date, " << col_info.column_ << ")"
			" SELECT date, " << col_info.column_ << " FROM " << col_info.schema_ << "." << col_info.table_ <<
			" ON CONFLICT (date) DO UPDATE SET " << col_info.column_ << " = excluded." << col_info.column_;
		// 3.
		const string col_path = col_info.to_string();
		impl_->sql_ <<
			"INSERT INTO " << dest_schema_name << ".meta_data (\"table\", \"column\", \"origin\", \"unit_id\")"
			" VALUES (:dest_table_name, :column, :col_path, :unit_id);"
			, use(dest_table_name, "dest_table_name"s)
			, use(col_info.column_, "column"s)
			, use(col_path, "col_path"s)
			, use(unit_id, "unit_id"s);
	}
	catch (const exception & ex)
	{
		throw runtime_error("Failed to load columns. "s + ex.what());
	}
}
//----------------------------------------------------------------------------------------------------------
void DbAccess::deleteColumn(const string& dest_schema_name, const string& dest_table_name, const ColumnMetaData& col_info)
{
	try
	{
		impl_->sql_ <<
			"ALTER TABLE " << dest_schema_name << "." << dest_table_name << " DROP COLUMN " << col_info.column_;
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
vector<UnitInfo> DbAccess::loadUnits() const
{
	static constexpr int id_idx = 0;
	static constexpr int name_idx = 1;

	vector<UnitInfo> result;

	QSqlQuery query("SELECT id, name FROM ready.units;");
	if (query.size() >= 0)
	{
		result.reserve(query.size());
	}

	while (query.next())
	{
		result.emplace_back(query.value(id_idx).toInt(), query.value(name_idx).toString().toStdString());
	}
	if (query.lastError().isValid())
	{
		throw runtime_error("Failed to load units. "s + query.lastError().text().toStdString());
	}
	return result;
}
//----------------------------------------------------------------------------------------------------------
vector<ColumnMetaData> DbAccess::loadMetaData() const
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

	vector<ColumnMetaData> result;

	try
	{
		rowset<row> rs = (impl_->sql_.prepare <<
		"SELECT id, \"table\", \"column\", description, origin, normalized, norm_min, norm_max, unit_id, date_min, date_max FROM ready.meta_data;");
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
		}
		return result;
	}
	catch (const exception& ex)
	{
		throw runtime_error("Failed to load meta data. "s + ex.what());
	}
}
//----------------------------------------------------------------------------------------------------------
void DbAccess::storeMetaData(const ColumnMetaData& col_info, const DataFrame& data) const
{
	try
	{
		vector<hmdf::DateTime>& idx = const_cast<vector<hmdf::DateTime>&>(data.get_index());

		impl_->sql_
			<< "UPDATE ready." << col_info.table_ << " SET " << col_info.column_ << " = :val WHERE date = :date"
			, use(data.get_column<double>(col_info.column_.c_str()), "val"s)
			, use(idx, "date"s);

		const double norm_min = col_info.normalized_ ? col_info.norm_min_ : 0.;
		const double norm_max = col_info.normalized_ ? col_info.norm_max_ : 0.;

		impl_->sql_
			<< "UPDATE ready.meta_data "
			"SET normalized = :normalized, norm_min = :norm_min, norm_max = :norm_max, date_min = :date_min, date_max = :date_max "
			"WHERE id = :id"
			, use(col_info.normalized_, "normalized")
			, use(norm_min, "norm_min")
			, use(norm_max, "norm_max")
			, use(col_info.id_, "id")
			, use(col_info.date_min_, "date_min")
			, use(col_info.date_max_, "date_max");
	}
	catch (const exception & ex)
	{
		throw runtime_error("Failed to store column data. "s + ex.what());
	}
}
//----------------------------------------------------------------------------------------------------------
// 1. count
// 2. index
// 3. data
DataFrame DbAccess::load_data(const string& schema, const string& table_name, const vector<string>& col_names) const
{
	DataFrame result;

	try
	{
		ptrdiff_t count = 0;
		// 1,
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
			vector<hmdf::DateTime> idx(count);
			impl_->sql_ << "SELECT date FROM " << schema << "." << table_name, into(idx);
			result.load_index(move(idx));
		}
		// 3.
		for (const string& col_name : col_names)
		{
			vector<double> col(count);
			impl_->sql_ << "SELECT " << col_name << " FROM " << schema << "." << table_name, into(col, ind);
			result.load_column(col_name.c_str(), move(col));
		}
		return result;
	}
	catch (const exception& ex)
	{
		throw runtime_error("Failed to load column data. "s + ex.what());
	}
}

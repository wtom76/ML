#include "stdafx.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDate>
#include "DbAccess.h"

//----------------------------------------------------------------------------------------------------------
DbAccess::DbAccess()
	: QObject(nullptr)
{
	QSqlDatabase db = QSqlDatabase::addDatabase(QString::fromStdString("QPSQL"));
	db.setHostName(QString::fromStdString("localhost"));
	db.setDatabaseName(QString::fromStdString("ML"));
	db.setUserName(QString::fromStdString("ml_user"));
	db.setPassword(QString::fromStdString("ml_user"));
	if (!db.open())
	{
		throw std::runtime_error(std::string("Failed to connect to database. ") +
			db.lastError().text().toStdString());
	}
}
//----------------------------------------------------------------------------------------------------------
DbAccess::~DbAccess()
{
}
//----------------------------------------------------------------------------------------------------------
std::set<QString> DbAccess::tableColumns(const QString& schema_name, const QString& table_name) const
{
	static constexpr int column_idx = 0;

	std::set<QString> result;

	QSqlQuery query(QString(
		"SELECT column_name FROM information_schema.columns "
		"WHERE table_schema = N'%1' AND table_name = N'%2'").arg(schema_name).arg(table_name)
	);
	while (query.next())
	{
		result.emplace(query.value(column_idx).toString());
	}
	if (query.lastError().isValid())
	{
		throw std::runtime_error(std::string("Failed to load columns. ") + query.lastError().text().toStdString());
	}
	return result;
}
//----------------------------------------------------------------------------------------------------------
void DbAccess::addColumn(const QString& dest_schema_name, const QString& dest_table_name, int unit_id,
	const ColumnPath& col_info)
{
	const QString query_str(QString(
		"ALTER TABLE %1.%2 ADD COLUMN %3 %4; "
		"INSERT INTO %1.%2 (date, %3) SELECT date, %3 FROM %6.%7 ON CONFLICT (date) DO UPDATE SET %3 = excluded.%3; "
		"INSERT INTO ready.meta_data (\"table\", \"column\", \"origin\", \"unit_id\") VALUES ('%2', '%3', '%5', %8);")
		.arg(dest_schema_name)			// 1
		.arg(dest_table_name)			// 2
		.arg(col_info.column_)			// 3
		.arg(col_info.data_type_)		// 4
		.arg(col_info.toString())		// 5
		.arg(col_info.schema_)			// 6
		.arg(col_info.table_)			// 7
		.arg(unit_id)					// 8
	);
	QSqlQuery query(query_str);
	if (query.lastError().isValid())
	{
		throw std::runtime_error(std::string("Failed to load sources. ") +
			query.lastError().text().toStdString());
	}
}
//----------------------------------------------------------------------------------------------------------
void DbAccess::deleteColumn(const QString& dest_schema_name, const QString& dest_table_name, const ColumnMetaData& col_info)
{
	const QString query_str(QString(
		"ALTER TABLE %1.%2 DROP COLUMN %3; "
		"DELETE FROM ready.meta_data WHERE id = %4;")
		.arg(dest_schema_name)			// 1
		.arg(dest_table_name)			// 2
		.arg(col_info.column_)			// 3
		.arg(col_info.id_)				// 4
	);
	QSqlQuery query(query_str);
	if (query.lastError().isValid())
	{
		throw std::runtime_error(std::string("Failed to load sources. ") +
			query.lastError().text().toStdString());
	}
}
//----------------------------------------------------------------------------------------------------------
std::vector<UnitInfo> DbAccess::loadUnits() const
{
	static constexpr int id_idx = 0;
	static constexpr int name_idx = 1;

	std::vector<UnitInfo> result;

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
		throw std::runtime_error(std::string("Failed to load units. ") + query.lastError().text().toStdString());
	}
	return result;
}
//----------------------------------------------------------------------------------------------------------
std::vector<ColumnMetaData> DbAccess::loadMetaData() const
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

	std::vector<ColumnMetaData> result;

	QSqlQuery query("SELECT id, \"table\", \"column\", description, origin, normalized, norm_min, norm_max, unit_id FROM ready.meta_data;");
	if (query.size() >= 0)
	{
		result.reserve(query.size());
	}

	while (query.next())
	{
		result.emplace_back(ColumnMetaData{});
		result.back().id_			= query.value(id_idx).toInt();
		result.back().table_		= query.value(table_idx).toString();
		result.back().column_		= query.value(column_idx).toString();
		result.back().description_	= query.value(description_idx).toString();
		result.back().origin_		= query.value(origin_idx).toString();
		result.back().normalized_	= query.value(normalized_idx).toBool();
		result.back().norm_min_		= query.value(norm_min_idx).toDouble();
		result.back().norm_max_		= query.value(norm_max_idx).toDouble();
		result.back().unit_id_		= query.value(unit_id_idx).toLongLong();
	}
	if (query.lastError().isValid())
	{
		throw std::runtime_error(std::string("Failed to load metadata. ") + query.lastError().text().toStdString());
	}
	return result;
}
//----------------------------------------------------------------------------------------------------------
ColumnData DbAccess::loadColumnData(const QString& schema, const QString& table, const QString& column) const
{
	static constexpr int date_idx	= 0;
	static constexpr int value_idx	= 1;

	ColumnData result;

	const QString query_str(QString("SELECT date, %1 FROM %2.%3")
		.arg(column)		// 1
		.arg(schema)		// 2
		.arg(table)			// 3
	);

	QSqlQuery query(query_str);
	if (query.size() >= 0)
	{
		result.dates_.reserve(query.size());
		result.values_.reserve(query.size());
		result.valid_.reserve(query.size());
	}
	while (query.next())
	{
		result.dates_.emplace_back(query.value(date_idx).toDate());
		result.valid_.emplace_back(!query.value(value_idx).isNull());
		if (query.value(value_idx).isNull())
		{
			result.values_.emplace_back(0);
		}
		else
		{
			result.values_.emplace_back(query.value(value_idx).toDouble());
		}
	}
	if (query.lastError().isValid())
	{
		throw std::runtime_error(std::string("Failed to load metadata. ") + query.lastError().text().toStdString());
	}
	return result;
}
//----------------------------------------------------------------------------------------------------------
void DbAccess::storeColumnData(const ColumnMetaData& col_info, const ColumnData& data) const
{
	{
		const QString query_str(QString("UPDATE ready.%1 SET %2 = :val WHERE date = :date")
			.arg(col_info.table_)		// 1
			.arg(col_info.column_)		// 2
		);
		QSqlQuery query;
		if (!query.prepare(query_str))
		{
			throw std::runtime_error(std::string("Failed to store data. ") + query.lastError().text().toStdString());
		}

		QVariantList date_list;
		QVariantList val_list;

		for (size_t i = 0; i < data.values_.size(); ++i)
		{
			if (data.valid_[i])
			{
				date_list.push_back(data.dates_[i]);
				val_list.push_back(data.values_[i]);
			}
		}
		query.bindValue(":date", date_list);
		query.bindValue(":val", val_list);
		if (!query.execBatch())
		{
			throw std::runtime_error(std::string("Failed to store metadata. ") + query.lastError().text().toStdString());
		}
	}

	{
		QSqlQuery query;;
		const QString query_str("UPDATE ready.meta_data SET normalized = :normalized, norm_min = :norm_min, norm_max = :norm_max WHERE id = :id");
		query.prepare(query_str);

		query.bindValue(":normalized", col_info.normalized_);
		query.bindValue(":norm_min", col_info.normalized_ ? col_info.norm_min_ : 0.);
		query.bindValue(":norm_max", col_info.normalized_ ? col_info.norm_max_ : 0.);
		query.bindValue(":id", col_info.id_);

		if (!query.exec())
		{
			throw std::runtime_error(std::string("Failed to store metadata. ") + query.lastError().text().toStdString());
		}
	}
}

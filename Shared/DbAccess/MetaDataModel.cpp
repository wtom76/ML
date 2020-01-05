#include "stdafx.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QApplication>
#include <Shared/DbAccess/DbAccess.h>
#include <Shared/Math/Normalization.hpp>
#include <Shared/Math/Feature.hpp>
#include "MetaDataModel.h"

using namespace std;
using namespace wtom::ml;

//----------------------------------------------------------------------------------------------------------
MetaDataModel::MetaDataModel(DbAccess& db, QObject* parent)
	: QAbstractTableModel(parent)
	, col_names_{"column", "table", "description", "origin", "normalized", "norm_min", "norm_max", "unit_id"}
	, db_(db)
{
	load();
}
//----------------------------------------------------------------------------------------------------------
MetaDataModel::~MetaDataModel()
{
}
//---------------------------------------------------------------------------------------------------------
void MetaDataModel::load()
{
	beginResetModel();
	data_ = db_.load_meta_data();
	endResetModel();
}
//---------------------------------------------------------------------------------------------------------
int MetaDataModel::rowCount(const QModelIndex& parent) const
{
	return parent.isValid() ? 0 : static_cast<int>(data_.size());
}
//---------------------------------------------------------------------------------------------------------
int MetaDataModel::columnCount(const QModelIndex& parent) const
{
	return parent.isValid() ? 0 : column_count_;
}
//---------------------------------------------------------------------------------------------------------
QVariant MetaDataModel::data(const QModelIndex& index, int role) const
{
	switch (role)
	{
	case Qt::DisplayRole:
		assert(index.column() < column_count_);
		{
			assert(index.row() < data_.size());
			switch (index.column())
			{
			case 0:	return QString::fromStdString(data_[index.row()].column_);
			case 1:	return QString::fromStdString(data_[index.row()].table_);
			case 2:	return QString::fromStdString(data_[index.row()].description_);
			case 3:	return QString::fromStdString(data_[index.row()].origin_);
			case 4:	return data_[index.row()].normalized_;
			case 5:	return data_[index.row()].norm_min_;
			case 6:	return data_[index.row()].norm_max_;
			case 7:	return data_[index.row()].unit_id_;
			}
		}
	}
	return QVariant();
}
//---------------------------------------------------------------------------------------------------------
QVariant MetaDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch (orientation)
	{
	case Qt::Horizontal:
		switch (role)
		{
		case Qt::DisplayRole:
			return col_names_[section];
		}
	}
	return QAbstractTableModel::headerData(section, orientation, role);
}
//----------------------------------------------------------------------------------------------------------
void MetaDataModel::add_column(const ColumnPath& path, int unit_id)
{
	set<string> std_existing_cols = db_.tableColumns(g_dest_schema, g_dest_table);
	if (std_existing_cols.find(path.column_) != std_existing_cols.cend())
	{
		return;
	}
	const ColumnMetaData meta{g_dest_table, path.column_, unit_id, path};
	db_.add_column(g_dest_schema, meta);
	db_.copy_column_data(g_dest_schema, g_dest_table, path);
	load();
}
//----------------------------------------------------------------------------------------------------------
void MetaDataModel::delete_column(int idx)
{
	assert(idx >= 0);
	assert(idx < data_.size());

	if (data_[idx].column_ == "date")
	{
		QMessageBox::warning(qApp->activeWindow(), "Column deletion is rejected", "'data' column should not be deleted");
		return;
	}
	db_.delete_column(g_dest_schema, g_dest_table, data_[idx]);
	load();
}
//----------------------------------------------------------------------------------------------------------
void MetaDataModel::normalize_column(math::NormalizationMethod method, int idx)
{
	assert(idx >= 0);
	assert(idx < data_.size());

	if (data_[idx].column_ == "date")
	{
		QMessageBox::warning(qApp->activeWindow(), "Column normalization is rejected", "'data' column can't be normalized");
		return;
	}
	if (data_[idx].normalized_)
	{
		QMessageBox::information(qApp->activeWindow(), QString("Column normalization is rejected"), QString("'%1' is already normalized").arg(data_[idx].column_.c_str()));
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	DataFrame col_data = load_column(idx);
	const math::Range min_max = math::min_max(col_data.data().front());
	switch (method)
	{
	case wtom::ml::math::NormalizationMethod::Sigmoid:
		math::normalize_range(min_max, math::Range{0., 1.}, col_data.data().front());
		break;
	case wtom::ml::math::NormalizationMethod::Tanh:
		math::normalize_tanh(min_max, col_data.data().front());
		break;
	default:
		assert(false);
	}
	data_[idx].normalized_ = true;
	data_[idx].norm_min_ = min_max.min();
	data_[idx].norm_max_ = min_max.max();
	db_.store_column(data_[idx], col_data);
	load();

	QApplication::restoreOverrideCursor();
	QMessageBox::information(qApp->activeWindow(), QString("Normalization result"), QString("'%1' is normalized").arg(data_[idx].column_.c_str()));
}
//----------------------------------------------------------------------------------------------------------
// TODO: load and store all non-norm cols at once
void MetaDataModel::normalize_all()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	int total_count = 0;
	int norm_count = 0;

	for (auto& col_info : data_)
	{
		++total_count;
		if (!col_info.normalized_ && col_info.column_ != "date")
		{
			DataFrame col_data = db_.load_data(g_dest_schema, col_info.table_, {col_info.column_});
			const math::Range min_max = math::min_max(col_data.data().front());
			math::normalize_range(min_max, math::Range{0., 1.}, col_data.data().front());
			col_info.normalized_ = true;
			col_info.norm_min_ = min_max.min();
			col_info.norm_max_ = min_max.max();
			db_.store_column(col_info, col_data);
			++norm_count;
		}
	}
	load();

	QApplication::restoreOverrideCursor();
	QMessageBox::information(qApp->activeWindow(), "Normalize all", QString("%1 normalized\n%2 processed").arg(norm_count).arg(total_count));
}
//----------------------------------------------------------------------------------------------------------
void MetaDataModel::make_target(int idx)
{
	constexpr size_t max_tolerated_gap = 3;

	assert(idx >= 0);
	assert(idx < data_.size());

	if (data_[idx].column_ == "date")
	{
		QMessageBox::warning(qApp->activeWindow(), "Making target is rejected", "'data' column can't be target origin");
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	ColumnMetaData target_meta{data_[idx]};
	target_meta.id_ = 0;
	target_meta.description_ = "next change in "s + target_meta.column_;
	target_meta.column_ += "_target";
	target_meta.normalized_ = false;
	target_meta.norm_max_ = target_meta.norm_min_ = numeric_limits<double>::quiet_NaN();

	DataFrame df = load_column(idx);
	vector<double>& target_col = *df.create_series(target_meta.column_, numeric_limits<double>::quiet_NaN());
	const vector<double>& src_col = *df.series(data_[idx].column_);
	wtom::ml::math::make_target_next_change(src_col, target_col, max_tolerated_gap);
	db_.add_column(g_dest_schema, target_meta);
	db_.store_column(target_meta, df);
	load();

	QApplication::restoreOverrideCursor();
	QMessageBox::information(qApp->activeWindow(), QString("Making target result"), QString("'%1' is created").arg(target_meta.column_.c_str()));
}
//----------------------------------------------------------------------------------------------------------
DataFrame MetaDataModel::load_column(int idx) const
{
	return db_.load_data(g_dest_schema, data_[idx].table_, {data_[idx].column_});
}
//----------------------------------------------------------------------------------------------------------
void MetaDataModel::store_column(int idx, const DataFrame& df) const
{
	db_.store_column(data_[idx], df);
}

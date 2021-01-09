#include "stdafx.h"
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QApplication>
#include <Shared/DbAccess/DbAccess.h>
#include <Shared/Math/Normalization.hpp>
#include <Shared/Math/Feature.hpp>
#include "MetaDataModel.h"

using namespace wtom::ml;

//----------------------------------------------------------------------------------------------------------
MetaDataModel::MetaDataModel(DbAccess& db, QObject* parent)
	: QAbstractTableModel(parent)
	, col_names_{"column", "table", "description", "origin", "normalized", "orig_min", "orig_max", "unit_id", "target"}
	, db_(db)
{
	// TEST
	//wtom::ml::math::test_make_delta();
	//~TEST

	load();
}
//----------------------------------------------------------------------------------------------------------
MetaDataModel::~MetaDataModel()
{
}
//----------------------------------------------------------------------------------------------------------
// QAbstractTableModel impl
//----------------------------------------------------------------------------------------------------------
int MetaDataModel::rowCount(const QModelIndex& parent) const
{
	return parent.isValid() ? 0 : static_cast<int>(data_.size());
}
//----------------------------------------------------------------------------------------------------------
int MetaDataModel::columnCount(const QModelIndex& parent) const
{
	return parent.isValid() ? 0 : column_count_;
}
//----------------------------------------------------------------------------------------------------------
Qt::ItemFlags MetaDataModel::flags(const QModelIndex& index) const
{
	switch (index.column())
	{
	case 0:
	case 8:
		return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
	default:
		return QAbstractItemModel::flags(index);
	}
}
//----------------------------------------------------------------------------------------------------------
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
		break;
	case Qt::CheckStateRole:
		switch (index.column())
		{
		case 0:
			return checked_[index.row()];
		case 8:
			return data_[index.row()].is_target_ ? Qt::Checked : Qt::Unchecked;
		}
		break;
	}

	return QVariant();
}
//----------------------------------------------------------------------------------------------------------
bool MetaDataModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	switch (role)
	{
	case Qt::CheckStateRole:
		switch (index.column())
		{
		case 0:
			checked_[index.row()] = static_cast<Qt::CheckState>(value.toInt());
			return true;
		case 8:
			data_[index.row()].is_target_ = value.toInt() == Qt::Checked;
			db_.store_column_metadata_is_target(data_[index.row()]);
			return true;
		}
		break;
	}
	return false;
}
//----------------------------------------------------------------------------------------------------------
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
//~QAbstractTableModel impl
//----------------------------------------------------------------------------------------------------------
void MetaDataModel::load()
{
	beginResetModel();
	data_ = db_.load_meta_data();
	checked_.resize(data_.size(), Qt::Unchecked);
	fill(checked_.begin(), checked_.end(), Qt::Unchecked);
	endResetModel();
}
//----------------------------------------------------------------------------------------------------------
void MetaDataModel::add_column(const ColumnPath& path, const std::string& dest_table, int unit_id, bool is_target, double target_error)
{
	assert(!dest_table.empty());
	set<string> std_existing_cols = db_.tableColumns(db_.dest_schema(), dest_table);
	if (std_existing_cols.find(path.column_) != std_existing_cols.cend())
	{
		return;
	}
	const ColumnMetaData meta{dest_table, path.column_, unit_id, path, is_target, target_error};
	db_.add_column(db_.dest_schema(), meta);
	db_.copy_column_data(db_.dest_schema(), dest_table, path);
	load();
}
//----------------------------------------------------------------------------------------------------------
void MetaDataModel::delete_checked_columns()
{
	for (size_t idx = 0; idx != checked_.size(); ++idx)
	{
		if (checked_[idx] == Qt::CheckState::Checked)
		{
			if (data_[idx].column_ == "date")
			{
				continue;
			}
			db_.delete_column(db_.dest_schema(), data_[idx]);
		}
	}
	load();
}
//----------------------------------------------------------------------------------------------------------
void MetaDataModel::normalize_checked_columns(math::NormalizationMethod method)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	for (size_t idx = 0; idx != checked_.size(); ++idx)
	{
		if (checked_[idx] != Qt::CheckState::Checked ||
			data_[idx].column_ == "date" ||
			data_[idx].normalized_)
		{
			continue;
		}

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
	}
	load();

	QApplication::restoreOverrideCursor();
	QMessageBox::information(qApp->activeWindow(), QString("Normalization"), QString("Done"));
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
			DataFrame col_data = db_.load_data(db_.dest_schema(), col_info.table_, {col_info.column_});
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
void MetaDataModel::_make_feature_delta(int idx, ptrdiff_t period, bool next)
{
	constexpr size_t max_tolerated_gap = 2;

	assert(idx >= 0);
	assert(idx < data_.size());

	if (data_[idx].column_ == "date")
	{
		QMessageBox::warning(qApp->activeWindow(), "Making feature is rejected", "'data' column can't be feature origin");
		return;
	}

	ColumnMetaData target_meta{ data_[idx] };
	target_meta.id_ = 0;
	target_meta.description_ = (next ? "next "s : ""s) + "delta in "s + target_meta.column_;
	target_meta.column_ += "_ft_delta_"s + to_string(period);
	if (next)
	{
		target_meta.column_ += "_next"s;
	}
	target_meta.normalized_ = false;
	target_meta.norm_max_ = target_meta.norm_min_ = numeric_limits<double>::quiet_NaN();

	DataFrame df = load_column(idx);
	vector<double>& target_col = *df.create_series(target_meta.column_, numeric_limits<double>::quiet_NaN());
	const vector<double>& src_col = df.series(data_[idx].column_);
	if (next)
	{
		wtom::ml::math::make_delta(src_col, target_col, max_tolerated_gap, period, 1);
	}
	else
	{
		wtom::ml::math::make_delta(src_col, target_col, max_tolerated_gap, period);
	}
	db_.add_column(db_.dest_schema(), target_meta);
	db_.store_column(target_meta, df);
}
//----------------------------------------------------------------------------------------------------------
void MetaDataModel::_make_feature_winloss(int idx, double treshold, bool next)
{
	assert(idx >= 0);
	assert(idx < data_.size());

	if (data_[idx].column_ == "date")
	{
		QMessageBox::warning(qApp->activeWindow(), "Making feature is rejected", "'data' column can't be feature origin");
		return;
	}

	ColumnMetaData target_meta{ data_[idx] };
	target_meta.id_ = 0;
	target_meta.description_ = (next ? "next "s : ""s) + "winloss in "s + target_meta.column_;
	target_meta.column_ += "_ft_winloss"s;
	if (next)
	{
		target_meta.column_ += "_next"s;
	}
	target_meta.normalized_ = false;
	target_meta.norm_max_ = target_meta.norm_min_ = numeric_limits<double>::quiet_NaN();

	DataFrame df = load_column(idx);
	vector<double>& target_col = *df.create_series(target_meta.column_, numeric_limits<double>::quiet_NaN());
	const vector<double>& src_col = df.series(data_[idx].column_);
	if (next)
	{
		wtom::ml::math::make_win_loss_close(src_col, target_col, treshold, 1);
	}
	else
	{
		wtom::ml::math::make_win_loss_close(src_col, target_col, treshold);
	}
	db_.add_column(db_.dest_schema(), target_meta);
	db_.store_column(target_meta, df);
}
//----------------------------------------------------------------------------------------------------------
void MetaDataModel::_make_feature_target_ohlc(const array<int, 4>& ohlc_idxs, double treshold, bool next)
{
#if !defined(NDEBUG)
	for (int i : ohlc_idxs)
	{
		assert(i >= 0);
		assert(i < data_.size());
	}
#endif

	ColumnMetaData target_meta{ data_[ohlc_idxs[0]] };
	target_meta.id_ = 0;
	target_meta.description_ = "OHLC win/loss";
	target_meta.column_ = "target_ohlc_winloss"s;
	if (next)
	{
		target_meta.column_ += "_next"s;
	}
	target_meta.normalized_ = false;
	target_meta.norm_max_ = target_meta.norm_min_ = numeric_limits<double>::quiet_NaN();

	DataFrame df = load_column(ohlc_idxs[0]);
	for (int i : {1, 2, 3})
	{
		df.append_equal(load_column(ohlc_idxs[i]));
	}

	vector<double>& target_col = *df.create_series(target_meta.column_, numeric_limits<double>::quiet_NaN());
	std::array<const vector<double>*, 4> ohlc_series{
		&df.series(data_[ohlc_idxs[0]].column_),
		&df.series(data_[ohlc_idxs[1]].column_),
		&df.series(data_[ohlc_idxs[2]].column_),
		&df.series(data_[ohlc_idxs[3]].column_)
	};
	wtom::ml::math::make_target_ohlc(ohlc_series, target_col, treshold, next ? 1 : 0);
	db_.add_column(db_.dest_schema(), target_meta);
	db_.store_column(target_meta, df);
}
//----------------------------------------------------------------------------------------------------------
void MetaDataModel::make_target_delta(int idx, ptrdiff_t period)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	_make_feature_delta(idx, period, true);

	load();
	QApplication::restoreOverrideCursor();
}
//----------------------------------------------------------------------------------------------------------
void MetaDataModel::make_target_winloss(int idx, double treshold)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	_make_feature_winloss(idx, treshold, true);

	load();
	QApplication::restoreOverrideCursor();
}
//----------------------------------------------------------------------------------------------------------
void MetaDataModel::make_target_ohlc(const std::array<int, 4>& ohlc_idxs, double treshold)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	_make_feature_target_ohlc(ohlc_idxs, treshold, true);

	load();
	QApplication::restoreOverrideCursor();
}
//----------------------------------------------------------------------------------------------------------
void MetaDataModel::make_feature_delta(int idx, ptrdiff_t period)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	_make_feature_delta(idx, period, false);

	load();
	QApplication::restoreOverrideCursor();
}
//----------------------------------------------------------------------------------------------------------
DataFrame MetaDataModel::load_column(size_t idx) const
{
	return db_.load_data(db_.dest_schema(), data_[idx].table_, {data_[idx].column_});
}
//----------------------------------------------------------------------------------------------------------
void MetaDataModel::store_column(size_t idx, const DataFrame& df) const
{
	db_.store_column(data_[idx], df);
}

#include "stdafx.h"
#include "TrainProgressModel.h"

//----------------------------------------------------------------------------------------------------------
TrainLogger* TrainProgressData::next()
{
	return owner_->create_entry();
}
//----------------------------------------------------------------------------------------------------------
// TrainLogger impl
//----------------------------------------------------------------------------------------------------------
void TrainProgressData::begin_teach(string label)
{
	{
		const auto write_lock();
		label_ = move(label);
	}
	owner_->invalidate(order_num_, 0);
}
//----------------------------------------------------------------------------------------------------------
void TrainProgressData::set_best(double cur_min_err)
{
	{
		const auto write_lock();
		current_min_err_ = cur_min_err;
	}
	owner_->invalidate(order_num_, 2);
}
//----------------------------------------------------------------------------------------------------------
void TrainProgressData::set_last(double err)
{
	{
		const auto write_lock();
		last_err_ = err;
	}
	owner_->invalidate(order_num_, 3);
}
//----------------------------------------------------------------------------------------------------------
void TrainProgressData::set_epoch(unsigned long long num)
{
	{
		const auto write_lock();
		epochs_passed_ = num;
	}
	owner_->invalidate(order_num_, 1);
}
//----------------------------------------------------------------------------------------------------------
void TrainProgressData::end_teach()
{
}
//~TrainLogger impl

//----------------------------------------------------------------------------------------------------------
// TrainProgressModel
//----------------------------------------------------------------------------------------------------------
TrainProgressModel::TrainProgressModel(QObject* parent)
	: QAbstractTableModel(parent)
	, col_names_{"label", "epochs passed", "current min err", "last err"}
    , timer_{new QTimer{this}}
{
    timer_->setInterval(1000);
    connect(timer_, &QTimer::timeout , this, &TrainProgressModel::update_all);
    timer_->start();
}
//----------------------------------------------------------------------------------------------------------
TrainProgressModel::~TrainProgressModel()
{
}
//----------------------------------------------------------------------------------------------------------
// QAbstractTableModel impl
//----------------------------------------------------------------------------------------------------------
int TrainProgressModel::rowCount(const QModelIndex& parent) const
{
	const auto entries_lock{data_entries_mtx_.write_lock()};
	return parent.isValid() ? 0 : static_cast<int>(data_.size());
}
//----------------------------------------------------------------------------------------------------------
int TrainProgressModel::columnCount(const QModelIndex& parent) const
{
	return parent.isValid() ? 0 : column_count_;
}
////----------------------------------------------------------------------------------------------------------
//Qt::ItemFlags TrainProgressModel::flags(const QModelIndex& index) const
//{
//	switch (index.column())
//	{
//	case 0:
//	case 8:
//		return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
//	default:
//		return QAbstractItemModel::flags(index);
//	}
//}
//----------------------------------------------------------------------------------------------------------
QVariant TrainProgressModel::data(const QModelIndex& index, int role) const
{
	switch (role)
	{
	case Qt::DisplayRole:
		assert(index.column() < column_count_);
		{
			assert(index.row() < data_.size());
			
			TrainProgressData* data{nullptr};
			{
				const auto entries_lock{data_entries_mtx_.write_lock()};
				data = data_[index.row()].get();
			}
			const auto lock{data->read_lock()};

			switch (index.column())
			{
			case 0:	return QString::fromStdString(data->label_);
			case 1:	return data->epochs_passed_;
			case 2:	return data->current_min_err_;
			case 3:	return data->last_err_;
			}
		}
		break;
	case Qt::CheckStateRole:
		break;
	}

	return QVariant();
}
////----------------------------------------------------------------------------------------------------------
//bool TrainProgressModel::setData(const QModelIndex& /*index*/, const QVariant& /*value*/, int role)
//{
//	switch (role)
//	{
//	case Qt::CheckStateRole:
//		break;
//	}
//	return false;
//}
//----------------------------------------------------------------------------------------------------------
QVariant TrainProgressModel::headerData(int section, Qt::Orientation orientation, int role) const
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
//~QAbstractTableModel impl
//----------------------------------------------------------------------------------------------------------
TrainLogger* TrainProgressModel::create_entry()
{
	TrainLogger* result{nullptr};
	{
		const auto lock{data_entries_mtx_.write_lock()};
		beginInsertRows({}, static_cast<int>(data_.size()), static_cast<int>(data_.size()));
		data_.emplace_back(make_unique<TrainProgressData>(static_cast<int>(data_.size()), this));
		result = data_.back().get();
	}
	//insertRows(rowCount(), 1);
	endInsertRows();
	return result;
}
//----------------------------------------------------------------------------------------------------------
void TrainProgressModel::clear()
{
	beginResetModel();
	{
		const auto lock{data_entries_mtx_.write_lock()};
		data_.clear();
	}
	endResetModel();
}
//----------------------------------------------------------------------------------------------------------
void TrainProgressModel::invalidate([[maybe_unused]] int row_idx, [[maybe_unused]] int col_idx)
{
	//static const QVector<int> roles{Qt::DisplayRole};
	//const QModelIndex cell_idx{createIndex(row_idx, col_idx)};
	//emit dataChanged(cell_idx, cell_idx, roles);
}
//----------------------------------------------------------------------------------------------------------
void TrainProgressModel::update_all()
{
	static const QVector<int> roles{Qt::DisplayRole};
	int rows{0};
	{
		const auto lock{data_entries_mtx_.write_lock()};
		rows = static_cast<int>(data_.size());
	}
	if (rows)
	{
		emit dataChanged(createIndex(0, 1), createIndex(static_cast<int>(rows - 1), static_cast<int>(column_count_ - 1)), roles);
	}
}

#include "stdafx.h"
#include <QAbstractTableModel>
#include <QDateTime>
#include "AdjustSplitsDlg.h"

//----------------------------------------------------------------------------------------------------------
long round_split_den(long den)
{
	return den > 100 ? (den / 10) * 10 : den;
}
//----------------------------------------------------------------------------------------------------------
std::vector<Split> detect_splits(const DataFrame& df, size_t col_idx)
{
	constexpr double threshold = 2.;
	std::vector<Split> result;
	const DataFrame::index_series_t& index = df.index();
	const DataFrame::series_t& series = df.data()[col_idx];
	if (series.empty())
	{
		return result;
	}
	double prev_val = series[0];
	for (size_t i = 1; i != series.size(); ++i)
	{
		if (!isnan(series[i]))
		{
			if (prev_val / series[i] >= threshold)
			{
				result.emplace_back(index[i], round_split_den(static_cast<long>(prev_val / series[i])));
			}
			prev_val = series[i];
		}
	}
	return result;
}
//----------------------------------------------------------------------------------------------------------
std::vector<Split> detect_splits_ohlc(const DataFrame& df)
{
	constexpr double threshold = 2.;
	constexpr double deviation_threshold = 0.01;
	std::vector<Split> result;

	const DataFrame::index_series_t& index{df.index()};
	const DataFrame::data_t& data{df.data()};
	if (index.size() < 2 || data.empty())
	{
		return result;
	}
	for (size_t i = 1; i != index.size(); ++i)
	{
		double split{0.};
		size_t split_count{0};
		for (size_t s = 0; s != data.size(); ++s)
		{
			const DataFrame::series_t& series{data[s]};
			if (!isnan(series[i]))
			{
				const double chg{series[i - 1] / series[i]};
				if (chg >= threshold && (split == 0. || abs(chg - split) / split < deviation_threshold))
				{
					split = split == 0. ? chg : (split * split_count + chg) / (split_count + 1);
					++split_count;
				}
			}
		}
		if (split_count == data.size())
		{
			result.emplace_back(index[i], round_split_den(static_cast<long>(split)));
		}
	}
	return result;
}
//----------------------------------------------------------------------------------------------------------
void apply_splits(const std::vector<Split>& splits, DataFrame& df, size_t col_idx)
{
	const DataFrame::index_series_t& index = df.index();
	DataFrame::series_t& series = df.data()[col_idx];
	for (const auto split : splits)
	{
		const auto idx_i = std::lower_bound(index.cbegin(), index.cend(), split.time_);
		assert(idx_i != index.cend());
		auto price_i = series.begin();
		auto price_e = next(price_i, distance(index.cbegin(), idx_i));
		for (; price_i != price_e; ++price_i)
		{
			*price_i = *price_i / split.den_;
		}
	}
}
//----------------------------------------------------------------------------------------------------------
void apply_splits_ohlc(const std::vector<Split>& splits, DataFrame& df)
{
	for (size_t i = 0; i != df.data().size(); ++i)
	{
		apply_splits(splits, df, i);
	}
}

//----------------------------------------------------------------------------------------------------------
SplitsModel::SplitsModel(const std::vector<Split>& data, QObject* parent)
	: QAbstractTableModel(parent)
	, col_names_{"time", "rate"}
	, data_{data}
{
}
//---------------------------------------------------------------------------------------------------------
int SplitsModel::rowCount(const QModelIndex& parent) const
{
	return parent.isValid() ? 0 : static_cast<int>(data_.size());
}
//---------------------------------------------------------------------------------------------------------
int SplitsModel::columnCount(const QModelIndex& parent) const
{
	return parent.isValid() ? 0 : column_count_;
}
//---------------------------------------------------------------------------------------------------------
QVariant SplitsModel::data(const QModelIndex& index, int role) const
{
	switch (role)
	{
	case Qt::DisplayRole:
		assert(index.column() < column_count_);
		{
			assert(index.row() < data_.size());
			switch (index.column())
			{
			case 0:	return QDateTime::fromTime_t(system_clock::to_time_t(data_[index.row()].time_)).toString("yyyy-MM-dd HH:mm:ss");
			case 1:	return QString::number(data_[index.row()].den_);
			}
		}
	}
	return QVariant();
}
//---------------------------------------------------------------------------------------------------------
QVariant SplitsModel::headerData(int section, Qt::Orientation orientation, int role) const
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
//---------------------------------------------------------------------------------------------------------
Qt::ItemFlags SplitsModel::flags(const QModelIndex& index) const
{
	return !index.isValid() ? Qt::ItemFlags{} : Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}
//---------------------------------------------------------------------------------------------------------
bool SplitsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (role != Qt::EditRole)
	{
		return false;
	}
	switch (index.column())
	{
	case 0:
		break;
	case 1:
		if (role == Qt::EditRole)
		{
			data_[index.row()].den_ = value.toInt();
			emit dataChanged(index, index, { role });
			return true;
		}
		break;
	}
	return false;
}
//---------------------------------------------------------------------------------------------------------
void SplitsModel::set(const std::vector<Split>& data)
{
	beginResetModel();
	data_ = data;
	endResetModel();
}

//---------------------------------------------------------------------------------------------------------
// class AdjustSplitsDlg
//---------------------------------------------------------------------------------------------------------
AdjustSplitsDlg::AdjustSplitsDlg(const std::vector<Split>& data, QWidget *parent)
	: QDialog{parent}
	, model_{make_unique<SplitsModel>(data, nullptr)}
{
	ui_.setupUi(this);
	ui_.splits_table_view_->setModel(model_.get());
}
//---------------------------------------------------------------------------------------------------------
AdjustSplitsDlg::~AdjustSplitsDlg()
{
}

#pragma once

#include <vector>
#include <array>
#include <QtCore/QAbstractTableModel>
#include <Shared/DbAccess/DbAccess.h>
#include <Shared/Math/Normalization.hpp>

//----------------------------------------------------------------------------------------------------------
// class MetaDataModel
//----------------------------------------------------------------------------------------------------------
class MetaDataModel : public QAbstractTableModel
{
	Q_OBJECT;

// data
private:
	static constexpr int column_count_ = 9;

	std::vector<ColumnMetaData> data_;
	std::vector<Qt::CheckState> checked_;
	std::array<QString, column_count_> col_names_;
	DbAccess& db_;

// methods
	void _make_feature_delta(int idx, ptrdiff_t period, bool next);
	void _make_feature_winloss(int idx, double treshold, bool next);
	void _make_feature_target_ohlc(const std::array<int, 4>& ohlc_idxs, double treshold, bool next);

public:
	MetaDataModel(DbAccess& db, QObject* parent);
	~MetaDataModel();

	// QAbstractTableModel impl
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	//~QAbstractTableModel impl

	void load();
	void add_column(const ColumnPath& path, const std::string& dest_table, int unit_id, bool is_target);
	void delete_checked_columns();
	void normalize_checked_columns(wtom::ml::math::NormalizationMethod method);
	void normalize_all();
	void make_target_delta(int idx, ptrdiff_t period);
	void make_target_winloss(int idx, double treshold);
	void make_target_ohlc(const std::array<int, 4>& ohlc_idxs, double treshold);
	void make_feature_delta(int idx, ptrdiff_t period);

	Qt::DropActions supportedDropActions() const override { return Qt::CopyAction; }

	std::vector<ColumnMetaData> columnInfos() const noexcept { return data_; } // copy?
	const ColumnMetaData& col_meta(int idx) const { return data_[idx]; }
	DataFrame load_column(size_t idx) const;
	void store_column(size_t idx, const DataFrame& df) const;
};

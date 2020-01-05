#pragma once

#include <vector>
#include <array>
#include <QAbstractTableModel>
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
	static constexpr int column_count_ = 8;

	std::vector<ColumnMetaData> data_;
	std::array<QString, column_count_> col_names_;
	DbAccess& db_;

// methods
public:
	MetaDataModel(DbAccess& db, QObject* parent);
	~MetaDataModel();

	// QAbstractTableModel impl
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	//~QAbstractTableModel impl

	void load();
	void add_column(const ColumnPath& path, int unit_id);
	void delete_column(int idx);
	void normalize_column(wtom::ml::math::NormalizationMethod method, int idx);
	void normalize_all();
	void make_target(int idx);

	Qt::DropActions supportedDropActions() const override { return Qt::CopyAction; }

	std::vector<ColumnMetaData> columnInfos() const noexcept { return data_; }
	DataFrame load_column(int idx) const;
	void store_column(int idx, const DataFrame& df) const;
};

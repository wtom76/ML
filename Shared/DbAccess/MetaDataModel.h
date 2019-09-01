#pragma once

#include <vector>
#include <array>
#include <QAbstractTableModel>
#include <Shared/DbAccess/DbAccess.h>

//----------------------------------------------------------------------------------------------------------
// class MetaDataModel
//----------------------------------------------------------------------------------------------------------
class MetaDataModel : public QAbstractTableModel
{
	Q_OBJECT;

// data
private:
	static constexpr int column_count_ = 8;

	std::vector<ColumnInfo> data_;
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
	void addColumn(const ColumnPath& path, int unit_id);
	void deleteColumn(int idx);

	Qt::DropActions supportedDropActions() const override { return Qt::CopyAction; }

	std::vector<ColumnInfo> columnInfos() const noexcept;
};

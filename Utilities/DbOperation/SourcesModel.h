#pragma once

#include <memory>
#include <QAbstractItemModel>
#include "TreeItem.h"

struct ColumnPath;

//---------------------------------------------------------------------------------------------------------
// enum class Fields
//---------------------------------------------------------------------------------------------------------
enum class Fields : int
{
	Schema = 0,
	Table,
	Column,
	DataType,
	FieldsCount
};

//-------------------------------------------------------------------------------------------------
template <typename EnumT>
constexpr typename std::underlying_type<EnumT>::type to_numeric(EnumT val) noexcept
{
	return static_cast<typename std::underlying_type<EnumT>::type>(val);
}

//----------------------------------------------------------------------------------------------------------
// class SourcesModel
//----------------------------------------------------------------------------------------------------------
class SourcesModel : public QAbstractItemModel
{
	Q_OBJECT;

	static constexpr int column_count_ = to_numeric(Fields::FieldsCount);

	std::unique_ptr<TreeItem> root_item_;

private:
	std::unique_ptr<TreeItem> _createRoot() const;
	TreeItem* _item(const QModelIndex& index) const;
	void _load();
	void _loadTableColumns(const QString table_name, TreeItem* table_item);
	void _buildPath(TreeItem* item, ColumnPath& dest) const;

public:
	SourcesModel(QObject* parent);
	~SourcesModel();

	void load();

	// read
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	QModelIndex index(int row, int column, const QModelIndex& parent_idx = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& index) const override;

	ColumnPath buildPath(const QModelIndex& index) const;

	// edit
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role = Qt::EditRole) override;
	bool insertRows(int row, int rows, const QModelIndex& parent = QModelIndex()) override;
	bool removeRows(int row, int rows, const QModelIndex& parent = QModelIndex()) override;

	Qt::DropActions supportedDragActions() const override { return Qt::CopyAction; }
};

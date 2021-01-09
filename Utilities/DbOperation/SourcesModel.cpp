#include "stdafx.h"
#include "SourcesModel.h"
#include <QSqlQuery>
#include <QSqlError>
#include <Shared/DbAccess/ColumnPath.h>
#include <Shared/LibIncludes/IncludeSoci.h>

using namespace soci;

//SELECT * FROM information_schema.columns
//WHERE table_schema = N'original';

//----------------------------------------------------------------------------------------------------------
SourcesModel::SourcesModel(QObject* parent)
	: QAbstractItemModel(parent)
{
}
//----------------------------------------------------------------------------------------------------------
SourcesModel::~SourcesModel()
{
}
//---------------------------------------------------------------------------------------------------------
std::unique_ptr<TreeItem> SourcesModel::_createRoot() const
{
	return std::make_unique<TreeItem>(std::initializer_list<QVariant>{
		QVariant("Schema"),
		QVariant("Table"),
		QVariant("Column"),
		QVariant("DataType")
	});
}
//---------------------------------------------------------------------------------------------------------
TreeItem* SourcesModel::_item(const QModelIndex& index) const
{
	if (index.isValid())
	{
		if (TreeItem* item = static_cast<TreeItem*>(index.internalPointer()))
		{
			return item;
		}
	}
	return root_item_.get();
}
//----------------------------------------------------------------------------------------------------------
void SourcesModel::_load(session& sql)
{
	static constexpr int table_schema_idx	= 0;
	static constexpr int table_name_idx		= 1;

	try
	{
		//string table_schema_str;
		//string table_name_str;
		//sql << "SELECT table_schema, table_name FROM information_schema.tables WHERE table_type = N'BASE TABLE' AND table_schema = N'original'",
		//	into(table_schema_str), into(table_name_str);

		rowset<row> rs = (sql.prepare <<
			"SELECT CAST(table_schema AS TEXT), CAST(table_name AS TEXT) FROM information_schema.tables "
			"WHERE table_type = N'BASE TABLE' AND table_schema = N'original';"
		);
		std::unique_ptr<TreeItem> root_item = _createRoot();
		for (const auto& row : rs)
		{
			const string table_name = row.get<string>(table_name_idx);
			std::unique_ptr<TreeItem> table_item = std::make_unique<TreeItem>(root_item.get());
			table_item->setData(to_numeric(Fields::Schema), QVariant{QString::fromStdString(row.get<string>(table_schema_idx))});
			table_item->setData(to_numeric(Fields::Table), QVariant{QString::fromStdString(table_name)});

			_load_table_columns(sql, table_name, table_item.get());

			root_item->appendChild(table_item.get());
			table_item.release();
		}
		root_item_.swap(root_item);
	}
	catch (const exception& ex)
	{
		throw runtime_error{"Failed to load sources. "s + ex.what()};
	}
}
//----------------------------------------------------------------------------------------------------------
void SourcesModel::_load_table_columns(session& sql, const string& table_name, TreeItem* table_item)
{
	static constexpr int column_idx		= 0;
	static constexpr int data_type_idx	= 1;

	try
	{
		rowset<row> rs = (sql.prepare <<
			"SELECT CAST(column_name AS TEXT), data_type FROM information_schema.columns "
			"WHERE table_schema = N'original' AND table_name = :table_name"
			, use(table_name, "table_name")
		);
		for (const auto& row : rs)
		{
			std::unique_ptr<TreeItem> column = std::make_unique<TreeItem>(table_item);
			column->setData(to_numeric(Fields::Column), QVariant{QString::fromStdString(row.get<string>(column_idx))});
			column->setData(to_numeric(Fields::DataType), QVariant{QString::fromStdString(row.get<string>(data_type_idx))});
			table_item->appendChild(column.get());
			column.release();
		}
	}
	catch (const exception& ex)
	{
		throw runtime_error{"Failed to load columns. "s + ex.what()};
	}
}
//----------------------------------------------------------------------------------------------------------
void SourcesModel::_build_path(TreeItem* item, ColumnPath& dest) const
{
	if (!item)
	{
		return;
	}
	QString val = item->data(to_numeric(Fields::Table), Qt::DisplayRole).toString();
	if (!val.isEmpty() && dest.table_.empty())
	{
		dest.table_ = val.toStdString();
	}
	val = item->data(to_numeric(Fields::Schema), Qt::DisplayRole).toString();
	if (!val.isEmpty() && dest.schema_.empty())
	{
		dest.schema_ = val.toStdString();
	}
	val = item->data(to_numeric(Fields::Column), Qt::DisplayRole).toString();
	if (!val.isEmpty() && dest.column_.empty())
	{
		dest.column_ = val.toStdString();
	}
	val = item->data(to_numeric(Fields::DataType), Qt::DisplayRole).toString();
	if (!val.isEmpty() && dest.data_type_.empty())
	{
		dest.data_type_ = val.toStdString();
	}
	_build_path(item->parent(), dest);
}
//----------------------------------------------------------------------------------------------------------
void SourcesModel::load(const string& connection_str)
{
	session sql{connection_str};

	beginResetModel();

	try
	{
		_load(sql);
	}
	catch (...)
	{
		endResetModel();
		throw;
	}

	endResetModel();
}
//---------------------------------------------------------------------------------------------------------
int SourcesModel::rowCount(const QModelIndex& parent) const
{
	return _item(parent)->childCount();
}
//---------------------------------------------------------------------------------------------------------
int SourcesModel::columnCount(const QModelIndex&) const
{
	return column_count_;
}
//---------------------------------------------------------------------------------------------------------
QVariant SourcesModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
	{
		return QVariant();
	}
	switch (role)
	{
	case Qt::DisplayRole:
	case Qt::EditRole:
		return _item(index)->data(index.column());
	}
	return QVariant();
}
//---------------------------------------------------------------------------------------------------------
QVariant SourcesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch (role)
	{
	case Qt::DisplayRole:
		switch (orientation)
		{
		case Qt::Horizontal:
			return root_item_->data(section);
		}
		break;
	}
	return QVariant();
}
//---------------------------------------------------------------------------------------------------------
QModelIndex SourcesModel::index(int row, int column, const QModelIndex& parent_idx) const
{
	if (!hasIndex(row, column, parent_idx))
	{
		return QModelIndex();
	}
	TreeItem* parent = parent_idx.isValid() ?
		static_cast<TreeItem*>(parent_idx.internalPointer()) : root_item_.get();

	TreeItem* child = parent->child(row);
	if (child)
	{
		return createIndex(row, column, child);
	}
	return QModelIndex();
}
//---------------------------------------------------------------------------------------------------------
QModelIndex SourcesModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
	{
		return QModelIndex();
	}

	TreeItem* parent_item = _item(index)->parent();

	if (root_item_.get() == parent_item)
	{
		return QModelIndex();
	}

	return createIndex(parent_item->childNumber(), 0, parent_item);
}
//---------------------------------------------------------------------------------------------------------
ColumnPath SourcesModel::buildPath(const QModelIndex& index) const
{
	ColumnPath path;
	_build_path(_item(index), path);
	return path;
}
//---------------------------------------------------------------------------------------------------------
Qt::ItemFlags SourcesModel::flags(const QModelIndex& index) const
{
	return !index.isValid() ? Qt::ItemFlags{} : QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled;
}
//---------------------------------------------------------------------------------------------------------
bool SourcesModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (role != Qt::EditRole)
	{
		return false;
	}
	if (_item(index)->setData(index.column(), value))
	{
		emit dataChanged(index, index, {role});
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------------------------------------
bool SourcesModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
	if (role != Qt::EditRole || orientation != Qt::Horizontal)
	{
		return false;
	}
	if (root_item_->setData(section, value))
	{
		emit headerDataChanged(orientation, section, section);
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------------------------------------
bool SourcesModel::insertRows(int row, int rows, const QModelIndex& parent)
{
	beginInsertRows(parent, row, row + rows - 1);
	const bool success = _item(parent)->insertChildren(row, rows);
	endInsertRows();

	return success;
}
//---------------------------------------------------------------------------------------------------------
bool SourcesModel::removeRows(int row, int rows, const QModelIndex& parent)
{
	beginRemoveRows(parent, row, row + rows - 1);
	const bool success = _item(parent)->removeChildren(row, rows);
	endRemoveRows();

	return success;
}

#include "stdafx.h"
#include "TreeItem.h"

//---------------------------------------------------------------------------------------------------------
// class TreeItem
//---------------------------------------------------------------------------------------------------------
// header item
TreeItem::TreeItem(std::initializer_list<QVariant> header)
	: parent_(nullptr)
{
	data_.assign(header);
	//std::copy(begin(header), end(header), begin(data_));
}
//---------------------------------------------------------------------------------------------------------
TreeItem::TreeItem(TreeItem* parent)
	: parent_(parent)
{
	data_.resize(parent->columnCount());
}
//---------------------------------------------------------------------------------------------------------
TreeItem::~TreeItem()
{
	qDeleteAll(children_);
}
//---------------------------------------------------------------------------------------------------------
bool TreeItem::setData(int column, const QVariant& value, int role)
{
	if (column < 0 || column >= columnCount())
	{
		return false;
	}
	switch (role)
	{
	case Qt::DisplayRole:
	case Qt::EditRole:
		data_[column] = value;
		break;
	case Qt::UserRole:
		user_data_[column] = value;
		break;
	default:
		return false;
	}
	return true;
}
//---------------------------------------------------------------------------------------------------------
QVariant TreeItem::data(int column, int role) const
{
	if (column < 0 || column >= columnCount())
	{
		return QVariant();
	}
	switch (role)
	{
	case Qt::DisplayRole:
	case Qt::EditRole:
		return data_[column];
	case Qt::UserRole:
		return user_data_[column];
	default:
		return QVariant();
	}
}
//---------------------------------------------------------------------------------------------------------
bool TreeItem::insertChildren(int position, int count)
{
	if (position < 0 || position > children_.size())
	{
		return false;
	}
	for (int row = 0; row < count; ++row)
	{
		children_.insert(position, new TreeItem(this));
	}
	return true;
}
//---------------------------------------------------------------------------------------------------------
void TreeItem::appendChild(TreeItem* child)
{
	assert(child->parent() == this);
	children_.append(child);
}
//---------------------------------------------------------------------------------------------------------
bool TreeItem::removeChildren(int position, int count)
{
	if (position < 0 || position + count > children_.size())
	{
		return false;
	}

	for (int row = 0; row < count; ++row)
	{
		delete children_.takeAt(position);
	}

	return true;
}
//---------------------------------------------------------------------------------------------------------
int TreeItem::childNumber() const noexcept
{
	if (parent_)
	{
		return parent_->children_.indexOf(const_cast<TreeItem*>(this));
	}
	return 0;
}

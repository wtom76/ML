#pragma once
#include <vector>
#include <QVariant>

//---------------------------------------------------------------------------------------------------------
// class TreeItem
//---------------------------------------------------------------------------------------------------------
class TreeItem
{
	std::vector<QVariant> data_;
	std::vector<QVariant> user_data_;
	TreeItem* parent_ = nullptr;
	QList<TreeItem*> children_;

public:
	explicit TreeItem(std::initializer_list<QVariant> header);			// absolute root ctor
	explicit TreeItem(TreeItem* parent);								// new level ctor
	~TreeItem();

	const QList<TreeItem*>& children() const noexcept { return children_; }
	TreeItem* child(int number) { return children_.value(number); }
	int childCount() const noexcept { return children_.size(); }
	int columnCount() const noexcept { return static_cast<int>(data_.size()); }
	bool insertChildren(int position, int count);
	void appendChild(TreeItem* child);
	TreeItem* parent() noexcept { return parent_; }
	bool removeChildren(int position, int count);
	int childNumber() const noexcept;
	bool setData(int column, const QVariant& value, int role = Qt::DisplayRole);
	QVariant data(int column, int role = Qt::DisplayRole) const;
};

#include "stdafx.h"
#include <QStringList>
#include "AddColumnDialog.h"
#include "DbOperation.h"
#include "ColumnMetrics.h"

//----------------------------------------------------------------------------------------------------------
AddColumnDialog::AddColumnDialog(QWidget* parent)
	: QDialog(parent)
{
	ui_.setupUi(this);

	_loadUnits();
}
//----------------------------------------------------------------------------------------------------------
AddColumnDialog::~AddColumnDialog()
{}
//----------------------------------------------------------------------------------------------------------
void AddColumnDialog::_loadUnits()
{
	DbAccess& db = DbOperation::instance()->db();
	std::vector<UnitInfo> units = db.loadUnits();
	QStringList names;
	for (const auto& unit : units)
	{
		names << QString::fromStdString(unit.name_);
	}
	ui_.units_->insertItems(0, names);
	for (int i = 0; i < units.size(); ++i)
	{
		ui_.units_->setItemData(i, QVariant{units[i].id_}, Qt::UserRole);
	}
}
//----------------------------------------------------------------------------------------------------------
void AddColumnDialog::setColumn(const ColumnPath& path)
{
	ui_.column_->setText(path.toString());
	const ColumnMetrics metrics(DbOperation::instance()->db().loadColumnData(path.schema_, path.table_, path.column_));
	ui_.min_->setText(QString::number(metrics.min_));
	ui_.max_->setText(QString::number(metrics.max_));
	ui_.nulls_->setText(QString::number(metrics.null_count_));
	ui_.rows_->setText(QString::number(metrics.valid_count_ + metrics.null_count_));
}
//----------------------------------------------------------------------------------------------------------
int AddColumnDialog::selectedUnitId() const
{
	const QVariant var = ui_.units_->currentData(Qt::UserRole);
	return var.isValid() ? var.toInt() : -1;
}

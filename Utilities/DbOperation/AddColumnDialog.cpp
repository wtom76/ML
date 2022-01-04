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
	_load_data_tables();
}
//----------------------------------------------------------------------------------------------------------
AddColumnDialog::~AddColumnDialog()
{}
//----------------------------------------------------------------------------------------------------------
void AddColumnDialog::_loadUnits()
{
	DbAccess& db{DbOperation::instance()->db()};
	std::vector<UnitInfo> units{db.load_units()};
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
void AddColumnDialog::_load_data_tables()
{
	DbAccess& db{DbOperation::instance()->db()};
	std::set<string> tables{db.data_tables()};
	QStringList names;
	for (const auto& table : tables)
	{
		names << QString::fromStdString(table);
	}
	ui_.dest_table_->insertItems(0, names);
}
//----------------------------------------------------------------------------------------------------------
void AddColumnDialog::setColumn(const ColumnPath& path)
{
	ui_.column_->setText(QString::fromStdString(path.to_string()));
	ui_.dest_column_->setText(QString::fromStdString(path.column_));
	DataFrame df = DbOperation::instance()->db().load_data(path.schema_, path.table_, {path.column_});
	const ColumnMetrics metrics(df.series(path.column_));
	ui_.min_->setText(QString::number(metrics.min_max_.min()));
	ui_.max_->setText(QString::number(metrics.min_max_.max()));
	ui_.nulls_->setText(QString::number(metrics.null_count_));
	ui_.rows_->setText(QString::number(metrics.valid_count_ + metrics.null_count_));
}
//----------------------------------------------------------------------------------------------------------
int AddColumnDialog::selectedUnitId() const
{
	const QVariant var = ui_.units_->currentData(Qt::UserRole);
	return var.isValid() ? var.toInt() : -1;
}
//----------------------------------------------------------------------------------------------------------
std::string AddColumnDialog::dest_table() const
{
	return ui_.dest_table_->currentText().toStdString();
}
//----------------------------------------------------------------------------------------------------------
std::string AddColumnDialog::dest_column() const
{
	return ui_.dest_column_->text().toStdString();
}
//----------------------------------------------------------------------------------------------------------
bool AddColumnDialog::is_target() const
{
	return ui_.is_target_->checkState() == Qt::Checked;
}
//----------------------------------------------------------------------------------------------------------
double AddColumnDialog::target_error() const
{
	return ui_.target_error_->text().toDouble();
}

#pragma once

#include <QtWidgets/QDialog>
#include "ui_AddColumnDialog.h"
#include "SourcesModel.h"

//----------------------------------------------------------------------------------------------------------
class AddColumnDialog : public QDialog
{
	Q_OBJECT;

private:
	Ui::AddColumnDialog ui_;

protected:
	void _loadUnits();

public:
	AddColumnDialog(QWidget* parent = nullptr);
	~AddColumnDialog();

	void setColumn(const ColumnPath& path);
	int selectedUnitId() const;
	std::string dest_table() const;
	bool is_target() const;
	double target_error() const;
};

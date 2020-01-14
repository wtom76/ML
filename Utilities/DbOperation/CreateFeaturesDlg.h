#pragma once

#include <QDialog>
#include "ui_CreateFeaturesDlg.h"

//----------------------------------------------------------------------------------------------------------
// class CreateFeaturesDlg
//----------------------------------------------------------------------------------------------------------
class CreateFeaturesDlg : public QDialog
{
	Q_OBJECT;

private:
	Ui::CreateFeaturesDlg ui_;

public:
	CreateFeaturesDlg(const QString& src_col_name, QWidget* parent = nullptr);
	~CreateFeaturesDlg();

	bool delta(int period = 1) const;
};

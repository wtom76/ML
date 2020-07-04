#pragma once
#include <string>

#include <QDialog>
#include "ui_DbParamsDlg.h"

class DbParamsDlg : public QDialog
{
	Q_OBJECT;

private:
	Ui::DbParamsDlg ui_;

public:
	DbParamsDlg(QWidget* parent = nullptr);
	~DbParamsDlg();

	std::string dest_schema() const;
};

#include "stdafx.h"
#include "DbParamsDlg.h"

//----------------------------------------------------------------------------------------------------------
DbParamsDlg::DbParamsDlg(QWidget* parent)
	: QDialog(parent)
{
	ui_.setupUi(this);
}
//----------------------------------------------------------------------------------------------------------
DbParamsDlg::~DbParamsDlg()
{
}
//----------------------------------------------------------------------------------------------------------
std::string DbParamsDlg::dest_schema() const
{
	return ui_.dest_schema_->currentText().toStdString();
}

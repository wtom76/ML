#include "stdafx.h"
#include "CreateFeaturesDlg.h"

//----------------------------------------------------------------------------------------------------------
CreateFeaturesDlg::CreateFeaturesDlg(const QString& src_col_name, QWidget* parent)
	: QDialog(parent)
{
	ui_.setupUi(this);
	ui_.src_col_name_->setText(src_col_name);
}
//----------------------------------------------------------------------------------------------------------
CreateFeaturesDlg::~CreateFeaturesDlg()
{
}
//----------------------------------------------------------------------------------------------------------
bool CreateFeaturesDlg::delta(int period) const
{
	switch (period)
	{
	case 1:
		return ui_.delta_->checkState() == Qt::CheckState::Checked;
	case 2:
		return ui_.delta_2->checkState() == Qt::CheckState::Checked;
	case 3:
		return ui_.delta_3->checkState() == Qt::CheckState::Checked;
	case 4:
		return ui_.delta_4->checkState() == Qt::CheckState::Checked;
	case 5:
		return ui_.delta_5->checkState() == Qt::CheckState::Checked;
	default:
		assert(false);
		return false;
	}
}

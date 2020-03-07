#include "stdafx.h"
#include "TargetOhlcSourceDlg.h"

//----------------------------------------------------------------------------------------------------------
TargetOhlcSourceDlg::TargetOhlcSourceDlg(const std::vector<std::pair<std::string, int>>& names, QWidget* parent)
	: QDialog(parent)
{
	ui_.setupUi(this);
	_populate_lists(names);
}
//----------------------------------------------------------------------------------------------------------
TargetOhlcSourceDlg::~TargetOhlcSourceDlg()
{
}
//----------------------------------------------------------------------------------------------------------
void TargetOhlcSourceDlg::_populate_lists(const std::vector<std::pair<std::string, int>>& names)
{
	for (const std::pair<std::string, int>& pr : names)
	{
		const QString q_name = QString::fromStdString(pr.first);
		ui_.name_o_->addItem(q_name, pr.second);
		ui_.name_h_->addItem(q_name, pr.second);
		ui_.name_l_->addItem(q_name, pr.second);
		ui_.name_c_->addItem(q_name, pr.second);
	}
}
//----------------------------------------------------------------------------------------------------------
int TargetOhlcSourceDlg::idx_o() const noexcept
{
	return ui_.name_o_->currentData().toInt();
}
//----------------------------------------------------------------------------------------------------------
int TargetOhlcSourceDlg::idx_h() const noexcept
{
	return ui_.name_h_->currentData().toInt();
}
//----------------------------------------------------------------------------------------------------------
int TargetOhlcSourceDlg::idx_l() const noexcept
{
	return ui_.name_l_->currentData().toInt();
}
//----------------------------------------------------------------------------------------------------------
int TargetOhlcSourceDlg::idx_c() const noexcept
{
	return ui_.name_c_->currentData().toInt();
}

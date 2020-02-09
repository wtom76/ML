#include "stdafx.h"
#include "MakeTargetDlg.h"

//---------------------------------------------------------------------------------------------------------
MakeTargetDlg::MakeTargetDlg(QWidget* parent)
	: QDialog(parent)
{
	ui_.setupUi(this);
	connect(ui_.type_, QOverload<const QString&>::of(&QComboBox::currentIndexChanged),
		[this](){ _on_type_changed(); });
}
//---------------------------------------------------------------------------------------------------------
MakeTargetDlg::~MakeTargetDlg()
{
}
//---------------------------------------------------------------------------------------------------------
MakeTargetDlg::Type MakeTargetDlg::type() const noexcept
{
	const QString text = ui_.type_->currentText();
	if (text == "Delta")
	{
		return Type::Delta;
	}
	else if (text == "WinLoss")
	{
		return Type::WinLoss;
	}
	else
	{
		return Type::Undefined;
	}
}
//---------------------------------------------------------------------------------------------------------
void MakeTargetDlg::_on_type_changed()
{
	const Type cur_type = type();
	ui_.period_->setEnabled(cur_type == Type::Delta);
	ui_.win_loss_threshold_->setEnabled(cur_type == Type::WinLoss);
}
//---------------------------------------------------------------------------------------------------------
ptrdiff_t MakeTargetDlg::period() const noexcept
{
	return ui_.period_->text().toInt();
}
//---------------------------------------------------------------------------------------------------------
double MakeTargetDlg::win_loss_treshold() const noexcept
{
	return ui_.win_loss_threshold_->text().toDouble();
}

#include "stdafx.h"
#include "NormalizationDlg.h"
#include <Shared/Utility/types.hpp>

//----------------------------------------------------------------------------------------------------------
NormalizationDlg::NormalizationDlg(QWidget* parent)
	: QDialog(parent)
{
	ui_.setupUi(this);

	ui_.method_box_->addItem("Sigmoid (0;1)", to_numeric(wtom::ml::math::NormalizationMethod::Sigmoid));
	ui_.method_box_->addItem("Tanh (-1;1)", to_numeric(wtom::ml::math::NormalizationMethod::Tanh));
}
//----------------------------------------------------------------------------------------------------------
NormalizationDlg::~NormalizationDlg()
{
}
//----------------------------------------------------------------------------------------------------------
wtom::ml::math::NormalizationMethod NormalizationDlg::method() const
{
	return static_cast<wtom::ml::math::NormalizationMethod>(ui_.method_box_->currentData().toInt());
}

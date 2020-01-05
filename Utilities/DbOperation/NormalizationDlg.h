#pragma once
#include <QDialog>
#include "ui_NormalizationDlg.h"
#include <Shared/Math/Statistics.hpp>
#include <Shared/Math/Normalization.hpp>

//----------------------------------------------------------------------------------------------------------
/// class NormalizationDlg
//----------------------------------------------------------------------------------------------------------
class NormalizationDlg : public QDialog
{
	Q_OBJECT;

private:
	Ui::NormalizationDlg ui_;

public:
	NormalizationDlg(QWidget* parent = nullptr);
	~NormalizationDlg();

	wtom::ml::math::NormalizationMethod method() const;
};

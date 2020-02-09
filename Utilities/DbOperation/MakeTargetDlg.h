#pragma once

#include <QDialog>
#include "ui_MakeTargetDlg.h"

//---------------------------------------------------------------------------------------------------------
/// class MakeTargetDlg
//---------------------------------------------------------------------------------------------------------
class MakeTargetDlg : public QDialog
{
	Q_OBJECT;

public:
	enum class Type
	{
		Undefined = 0,
		Delta,
		WinLoss
	};

private:
	Ui::MakeTargetDlg ui_;

private slots:
	void _on_type_changed();

public:
	MakeTargetDlg(QWidget* parent = nullptr);
	~MakeTargetDlg();

	Type type() const noexcept;
	ptrdiff_t period() const noexcept;
	double win_loss_treshold() const noexcept;
};

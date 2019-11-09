#pragma once

#include <QDialog>
#include "ui_CorrelationsDlg.h"

//----------------------------------------------------------------------------------------------------------
// class CorrelationsDlg
//----------------------------------------------------------------------------------------------------------
class CorrelationsDlg : public QDialog
{
	Q_OBJECT;

private:
	Ui::CorrelationsDlg ui_;

public:
	CorrelationsDlg(QWidget* parent = nullptr);
	~CorrelationsDlg();

	QTableView* view() noexcept { return ui_.correlations_view_; }
};

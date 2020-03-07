#pragma once

#include <vector>
#include <string>
#include <QDialog>
#include "ui_TargetOhlcSourceDlg.h"

//----------------------------------------------------------------------------------------------------------
/// class TargetOhlcSourceDlg
//----------------------------------------------------------------------------------------------------------
class TargetOhlcSourceDlg : public QDialog
{
	Q_OBJECT;

private:
	Ui::TargetOhlcSourceDlg ui_;

private:
	void _populate_lists(const std::vector<std::pair<std::string, int>>& names);

public:
	TargetOhlcSourceDlg(const std::vector<std::pair<std::string, int>>& names, QWidget* parent = nullptr);
	~TargetOhlcSourceDlg();

	int idx_o() const noexcept;
	int idx_h() const noexcept;
	int idx_l() const noexcept;
	int idx_c() const noexcept;
};

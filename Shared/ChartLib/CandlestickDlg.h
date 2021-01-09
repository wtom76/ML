#pragma once

#include <QDialog>
#include "ui_CandlestickDlg.h"

namespace chart
{
	//----------------------------------------------------------------------------------------------------------
	/// class CandlestickDlg
	//----------------------------------------------------------------------------------------------------------
	class CandlestickDlg : public QDialog
	{
		Q_OBJECT;

	private:
		Ui::CandlestickDlg ui_;

	public:
		CandlestickDlg(QWidget* parent = nullptr);
		~CandlestickDlg();

		string schema() const	{ return ui_.schema_->currentText().toStdString(); }
		string table() const	{ return ui_.table_->text().toStdString(); }
		string open_col() const	{ return ui_.open_col_->text().toStdString(); }
		string high_col() const	{ return ui_.high_col_->text().toStdString(); }
		string low_col() const	{ return ui_.low_col_->text().toStdString(); }
		string close_col() const{ return ui_.close_col_->text().toStdString(); }

		friend void to_json(json& j, const CandlestickDlg& src);
		friend void from_json(const json& j, CandlestickDlg& dst);
	};
	std::ostream& operator << (std::ostream& strm, const CandlestickDlg& dlg);
	std::istream& operator >> (std::istream& strm, CandlestickDlg& dlg);
}

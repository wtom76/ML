#pragma once

#include <QDialog>
#include "ui_line_dlg.h"

namespace chart
{
	//---------------------------------------------------------------------------------------------------------
	/// class line_dlg
	//---------------------------------------------------------------------------------------------------------
	class line_dlg : public QDialog
	{
		Q_OBJECT;

	private:
		Ui::line_dlg ui_;

	public:
		line_dlg(QWidget* parent = nullptr);
		~line_dlg();

		string schema() const		{ return ui_.schema_->currentText().toStdString(); }
		string table() const		{ return ui_.table_->text().toStdString(); }
		string values_col() const	{ return ui_.values_col_->text().toStdString(); }

		friend void to_json(json& j, const line_dlg& src);
		friend void from_json(const json& j, line_dlg& dst);
	};
	std::ostream& operator << (std::ostream& strm, const line_dlg& dlg);
	std::istream& operator >> (std::istream& strm, line_dlg& dlg);
}
#include "stdafx.h"
#include "line_dlg.h"

namespace chart
{
	//---------------------------------------------------------------------------------------------------------
	line_dlg::line_dlg(QWidget* parent)
		: QDialog(parent)
	{
		ui_.setupUi(this);
	}
	//---------------------------------------------------------------------------------------------------------
	line_dlg::~line_dlg()
	{
	}
	//---------------------------------------------------------------------------------------------------------
	void to_json(json& j, const line_dlg& src)
	{
		j = json{
			{ "schema", src.ui_.schema_->currentText().toStdString() },
			{ "table", src.ui_.table_->text().toStdString() },
			{ "date_col", src.ui_.date_col_->text().toStdString() },
			{ "values_col", src.ui_.values_col_->text().toStdString() }
		};
	}
	//---------------------------------------------------------------------------------------------------------
	void from_json(const json& j, line_dlg& dst)
	{
		dst.ui_.schema_->setCurrentText(QString::fromStdString(j.at("schema").get<std::string>()));
		dst.ui_.table_->setText(QString::fromStdString(j.at("table").get<std::string>()));
		dst.ui_.date_col_->setText(QString::fromStdString(j.at("date_col").get<std::string>()));
		dst.ui_.values_col_->setText(QString::fromStdString(j.at("values_col").get<std::string>()));
	}
	//---------------------------------------------------------------------------------------------------------
	std::ostream& operator <<(std::ostream& s, const chart::line_dlg& cfg)
	{
		return s << setw(4) << json{ { "line", cfg } };
	}
	//---------------------------------------------------------------------------------------------------------
	std::istream& operator >>(std::istream& s, chart::line_dlg& cfg)
	{
		json j;
		s >> j;
		chart::from_json(j.at("line"), cfg);
		return s;
	}
}

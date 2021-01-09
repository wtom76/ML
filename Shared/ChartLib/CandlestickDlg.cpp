#include "stdafx.h"
#include "CandlestickDlg.h"

namespace chart
{
	//----------------------------------------------------------------------------------------------------------
	CandlestickDlg::CandlestickDlg(QWidget* parent)
		: QDialog(parent)
	{
		ui_.setupUi(this);
	}
	//----------------------------------------------------------------------------------------------------------
	CandlestickDlg::~CandlestickDlg()
	{
	}
	//---------------------------------------------------------------------------------------------------------
	void to_json(json& j, const CandlestickDlg& src)
	{
		j = json{
			{ "schema", src.ui_.schema_->currentText().toStdString() },
			{ "table", src.ui_.table_->text().toStdString() },
			{ "date_col", src.ui_.date_col_->text().toStdString() },
			{ "open_col", src.ui_.open_col_->text().toStdString() },
			{ "high_col", src.ui_.high_col_->text().toStdString() },
			{ "low_col", src.ui_.low_col_->text().toStdString() },
			{ "close_col", src.ui_.close_col_->text().toStdString() }
		};
	}
	//---------------------------------------------------------------------------------------------------------
	void from_json(const json& j, CandlestickDlg& dst)
	{
		dst.ui_.schema_->setCurrentText(QString::fromStdString(j.at("schema").get<std::string>()));
		dst.ui_.table_->setText(QString::fromStdString(j.at("table").get<std::string>()));
		dst.ui_.date_col_->setText(QString::fromStdString(j.at("date_col").get<std::string>()));
		dst.ui_.open_col_->setText(QString::fromStdString(j.at("open_col").get<std::string>()));
		dst.ui_.high_col_->setText(QString::fromStdString(j.at("high_col").get<std::string>()));
		dst.ui_.low_col_->setText(QString::fromStdString(j.at("low_col").get<std::string>()));
		dst.ui_.close_col_->setText(QString::fromStdString(j.at("close_col").get<std::string>()));
	}
	//---------------------------------------------------------------------------------------------------------
	std::ostream& operator <<(std::ostream& s, const chart::CandlestickDlg& cfg)
	{
		return s << setw(4) << json{ { "Candlestick", cfg } };
	}
	//---------------------------------------------------------------------------------------------------------
	std::istream& operator >>(std::istream& s, chart::CandlestickDlg& cfg)
	{
		json j;
		s >> j;
		chart::from_json(j.at("Candlestick"), cfg);
		return s;
	}
}

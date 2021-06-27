#include "stdafx.h"
#include "OhlcDlg.h"
#include "ui_OhlcDlg.h"

//----------------------------------------------------------------------------------------------------------
OhlcDlg::OhlcDlg(QWidget* parent)
	: QDialog(parent)
	, ui_{make_unique<Ui::OhlcDlg>()}
{
	ui_->setupUi(this);
}
//----------------------------------------------------------------------------------------------------------
OhlcDlg::~OhlcDlg()
{
}
//---------------------------------------------------------------------------------------------------------
string OhlcDlg::schema()		const	{ return ui_->schema_->currentText().toStdString();	}
string OhlcDlg::table()			const	{ return ui_->table_->text().toStdString();			}
string OhlcDlg::open_col()		const	{ return ui_->open_col_->text().toStdString();		}
string OhlcDlg::high_col()		const	{ return ui_->high_col_->text().toStdString();		}
string OhlcDlg::low_col()		const	{ return ui_->low_col_->text().toStdString();		}
string OhlcDlg::close_col()		const	{ return ui_->close_col_->text().toStdString();		}
string OhlcDlg::volume_col()	const	{ return ui_->volume_col_->text().toStdString();	}
//---------------------------------------------------------------------------------------------------------
void to_json(json& j, const OhlcDlg& src)
{
	j = json{
		{ "schema",		src.ui_->schema_->currentText().toStdString()	},
		{ "table",		src.ui_->table_->text().toStdString()			},
		{ "date_col",	src.ui_->date_col_->text().toStdString()		},
		{ "open_col",	src.ui_->open_col_->text().toStdString()		},
		{ "high_col",	src.ui_->high_col_->text().toStdString()		},
		{ "low_col",	src.ui_->low_col_->text().toStdString()			},
		{ "close_col",	src.ui_->close_col_->text().toStdString()		}
	};
}
//---------------------------------------------------------------------------------------------------------
void from_json(const json& j, OhlcDlg& dst)
{
	dst.ui_->schema_->setCurrentText(QString::fromStdString(j.at("schema").get<std::string>()));
	dst.ui_->table_->setText(QString::fromStdString(j.at("table").get<std::string>()));
	dst.ui_->date_col_->setText(QString::fromStdString(j.at("date_col").get<std::string>()));
	dst.ui_->open_col_->setText(QString::fromStdString(j.at("open_col").get<std::string>()));
	dst.ui_->high_col_->setText(QString::fromStdString(j.at("high_col").get<std::string>()));
	dst.ui_->low_col_->setText(QString::fromStdString(j.at("low_col").get<std::string>()));
	dst.ui_->close_col_->setText(QString::fromStdString(j.at("close_col").get<std::string>()));
}
//---------------------------------------------------------------------------------------------------------
std::ostream& operator <<(std::ostream& s, const OhlcDlg& cfg)
{
	return s << setw(4) << json{ { "OHLC", cfg } };
}
//---------------------------------------------------------------------------------------------------------
std::istream& operator >>(std::istream& s, OhlcDlg& cfg)
{
	json j;
	s >> j;
	from_json(j.at("OHLC"), cfg);
	return s;
}

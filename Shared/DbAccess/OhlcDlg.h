#pragma once

#include <QDialog>

namespace Ui
{
	class OhlcDlg;
}

//----------------------------------------------------------------------------------------------------------
/// class OhlcDlg
//----------------------------------------------------------------------------------------------------------
class OhlcDlg : public QDialog
{
	Q_OBJECT;

private:
	unique_ptr<Ui::OhlcDlg> ui_;

public:
	OhlcDlg(QWidget* parent = nullptr);
	~OhlcDlg();

	string schema()		const;
	string table()		const;
	string open_col()	const;
	string high_col()	const;
	string low_col()	const;
	string close_col()	const;
	string volume_col()	const;

	friend void to_json(json& j, const OhlcDlg& src);
	friend void from_json(const json& j, OhlcDlg& dst);
};
std::ostream& operator << (std::ostream& strm, const OhlcDlg& dlg);
std::istream& operator >> (std::istream& strm, OhlcDlg& dlg);

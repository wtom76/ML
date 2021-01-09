#include "stdafx.h"
#include "DbOperation.h"
#include "DbParamsDlg.h"
#include <QtWidgets/QApplication>
#include <QMessageBox>

shared_ptr<spdlog::logger> g_log;

//----------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	util::SpdlogAsyncInit logger_init;
	g_log = util::create_logger("log");
	QApplication a(argc, argv);
	try
	{
		DbParamsDlg dlg;
		if (dlg.exec() != QDialog::Accepted)
		{
			return 0;
		}
		DbOperation w;
		w.db().set_dest_schema(dlg.dest_schema());
		w.show();
		return a.exec();
	}
	catch (std::exception& ex)
	{
		QMessageBox::critical(nullptr, "DbOperation: critical error", QString::fromStdString(ex.what()));
	}
}

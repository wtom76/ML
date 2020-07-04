#include "stdafx.h"
#include "DbOperation.h"
#include "DbParamsDlg.h"
#include <QtWidgets/QApplication>
#include <QMessageBox>

//----------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
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
		//w.db().set_dest_table(dlg.dest_table());
		w.show();
		return a.exec();
	}
	catch (std::exception& ex)
	{
		QMessageBox::critical(nullptr, "DbOperation: critical error", QString::fromStdString(ex.what()));
	}
}

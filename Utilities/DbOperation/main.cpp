#include "stdafx.h"
#include "DbOperation.h"
#include <QtWidgets/QApplication>
#include <QMessageBox>

//----------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	try
	{
		DbOperation w;
		w.show();
		return a.exec();
	}
	catch (std::exception& ex)
	{
		QMessageBox::critical(nullptr, "DbOperation: critical error", QString::fromStdString(ex.what()));
	}
}

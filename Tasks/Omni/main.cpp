#include "stdafx.h"
#include "Omni.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	try
	{
		Omni w;
		w.show();
		return a.exec();
	}
	catch (const std::exception& ex)
	{
		QMessageBox::critical(nullptr, "Omni error", QString::fromStdString(ex.what()));
	}
}

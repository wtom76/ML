#include "stdafx.h"
#include "Omni.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

int main(int argc, char *argv[])
{
	util::SpdlogAsyncInit logger_init;
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

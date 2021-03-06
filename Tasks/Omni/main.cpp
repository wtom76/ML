#include "stdafx.h"
#include "Omni.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

int main(int argc, char *argv[])
{
	util::SpdlogAsyncInit logger_init;
	shared_ptr<spdlog::logger> log{util::create_logger("log")};
	SPDLOG_LOGGER_DEBUG(log, "Omni main test {:s}", "test string"s);
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

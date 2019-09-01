#include "stdafx.h"
#include "Mbs.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Mbs w;
	w.show();
	return a.exec();
}

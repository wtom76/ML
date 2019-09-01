#include "stdafx.h"
#include "Mbs.h"
#include "DbAccess.h"

Mbs::Mbs(QWidget* parent)
	: QMainWindow(parent)
	, data_frame_(make_unique<ml::Data::Frame<double>>())
{
	ui_.setupUi(this);

	DbAccess db;
	db.load(*data_frame_);
}

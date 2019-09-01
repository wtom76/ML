#pragma once
#include <memory>
#include <Shared/Data/Frame.hpp>

#include <QtWidgets/QMainWindow>
#include "ui_Mbs.h"

using namespace std;
using namespace wtom;

class Mbs : public QMainWindow
{
	Q_OBJECT;

private:
	Ui::MbsClass ui_;
	unique_ptr<ml::Data::Frame<double>> data_frame_;

public:
	Mbs(QWidget* parent = Q_NULLPTR);
};

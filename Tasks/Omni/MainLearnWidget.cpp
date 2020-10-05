#include "stdafx.h"
#include "MainLearnWidget.h"
#include "TrainingSupervision.h"

//----------------------------------------------------------------------------------------------------------
MainLearnWidget::MainLearnWidget(QWidget* parent)
	: QWidget{parent}
	, train_supervisor_{make_unique<TrainingSupervision>()}
{
	ui_.setupUi(this);

	connect(ui_.start_btn_, &QPushButton::clicked, this, &MainLearnWidget::slot_start);
	connect(ui_.cancel_btn_, &QPushButton::clicked, this, &MainLearnWidget::slot_cancel);
}
//----------------------------------------------------------------------------------------------------------
MainLearnWidget::~MainLearnWidget()
{
}
//----------------------------------------------------------------------------------------------------------
void MainLearnWidget::slot_start()
{
	train_supervisor_->start();
}
//----------------------------------------------------------------------------------------------------------
void MainLearnWidget::slot_cancel()
{
	train_supervisor_->stop();
}

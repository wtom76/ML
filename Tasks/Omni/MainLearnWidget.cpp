#include "stdafx.h"
#include "MainLearnWidget.h"
#include "TrainingSupervision.h"
#include <Tasks/Omni/TrainingTask/MfnProbeContext.h>
#include <Tasks/Omni/TrainingTask/MfnProbe.h>

//----------------------------------------------------------------------------------------------------------
MainLearnWidget::MainLearnWidget(shared_ptr<DbAccess> db, QWidget* parent)
	: QWidget{parent}
	, db_{std::move(db)}
	, train_supervisor_{make_unique<TrainingSupervision>()}
	, mfn_ctx_{make_shared<training_task::MfnProbeContext>(db_)}
{
	ui_.setupUi(this);

	connect(ui_.start_btn_, &QPushButton::clicked, this, &MainLearnWidget::slot_start);
	connect(ui_.cancel_btn_, &QPushButton::clicked, this, &MainLearnWidget::slot_cancel);

	// temp. types should be choosen in GUI
	train_supervisor_->init_tasks(make_shared<training_task::MfnProbe>(mfn_ctx_));
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

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
	, train_pogress_model_{make_unique<TrainProgressModel>(this)}
{
	ui_.setupUi(this);

	ui_.train_progress_view_->setModel(train_pogress_model_.get());
	ui_.target_error_->setText(QString::number(mfn_ctx_->target_error()));

	connect(ui_.start_btn_, &QPushButton::clicked, this, &MainLearnWidget::slot_start);
	connect(ui_.cancel_btn_, &QPushButton::clicked, this, &MainLearnWidget::slot_cancel);
}
//----------------------------------------------------------------------------------------------------------
MainLearnWidget::~MainLearnWidget()
{
	train_supervisor_.reset();
}
//----------------------------------------------------------------------------------------------------------
void MainLearnWidget::slot_start()
{
	train_pogress_model_->clear();
	mfn_ctx_->target_error() = ui_.target_error_->text().toDouble();
	// temp. types should be choosen in GUI
	train_supervisor_->init_tasks(make_shared<training_task::MfnProbe>(mfn_ctx_, train_supervisor_->run_flag(), train_pogress_model_->create_entry()));
	train_supervisor_->start();
}
//----------------------------------------------------------------------------------------------------------
void MainLearnWidget::slot_cancel()
{
	train_supervisor_->stop();
}

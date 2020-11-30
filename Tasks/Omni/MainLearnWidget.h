#pragma once

#include <memory>
#include <QWidget>
#include "ui_MainLearnWidget.h"
#include "TrainProgressModel.h"

class TrainingSupervision;
class DbAccess;

namespace training_task
{
	class MfnProbeContext;
}

//----------------------------------------------------------------------------------------------------------
class MainLearnWidget : public QWidget
{
	Q_OBJECT;

private:
	Ui::MainLearnWidget							ui_;
	shared_ptr<DbAccess>						db_;
	unique_ptr<TrainingSupervision>				train_supervisor_;
	shared_ptr<training_task::MfnProbeContext>	mfn_ctx_;
	unique_ptr<TrainProgressModel>				train_pogress_model_;

public:
	MainLearnWidget(shared_ptr<DbAccess> db, QWidget* parent = nullptr);
	~MainLearnWidget();

private slots:
	void slot_start();
	void slot_cancel();
};

#pragma once

#include <memory>
#include <QWidget>
#include "ui_MainLearnWidget.h"

class TrainingSupervision;

//----------------------------------------------------------------------------------------------------------
class MainLearnWidget : public QWidget
{
	Q_OBJECT;

private:
	Ui::MainLearnWidget ui_;
	unique_ptr<TrainingSupervision> train_supervisor_;

public:
	MainLearnWidget(QWidget* parent = nullptr);
	~MainLearnWidget();

private slots:
	void slot_start();
	void slot_cancel();
};

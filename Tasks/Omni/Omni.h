#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QDockWidget>
#include <QtCharts/QChart>
#include <Shared/DbAccess/DbAccess.h>
#include <Shared/DbAccess/MetaDataModel.h>
#include "MainLearnWidget.h"
#include "ui_Omni.h"

//----------------------------------------------------------------------------------------------------------
class Omni : public QMainWindow
{
	Q_OBJECT;

private:
	Ui::OmniClass				ui_;
	shared_ptr<DbAccess>		db_;
	unique_ptr<MetaDataModel>	metadata_model_;
	unique_ptr<MainLearnWidget>	main_learn_proc_;

private:
	void _createMetadataView();
	QtCharts::QChart* _createChart();
	void _createChartView();
	void _createMainLearnProcWidget();

public:
	Omni(QWidget* parent = nullptr);

public slots:
	void showCorrelations();
	void showLearnMfn();
};

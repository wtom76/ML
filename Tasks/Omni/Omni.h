#pragma once

#include <QtWidgets/QMainWindow>
#include <QtCharts/QChart>
#include <Shared/DbAccess/DbAccess.h>
#include <Shared/DbAccess/MetaDataModel.h>
#include "ui_Omni.h"

//----------------------------------------------------------------------------------------------------------
class Omni : public QMainWindow
{
	Q_OBJECT;

private:
	Ui::OmniClass ui_;
	std::shared_ptr<DbAccess>		db_;
	std::unique_ptr<MetaDataModel>	metadata_model_;

private:
	void _createMetadataView();
	QtCharts::QChart* _createChart();
	void _createChartView();

public:
	Omni(QWidget* parent = nullptr);

public slots:
	void showCorrelations();
	void showLearnMfn();
};

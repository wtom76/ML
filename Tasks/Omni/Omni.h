#pragma once

#include <QtWidgets/QMainWindow>
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

public:
	Omni(QWidget* parent = nullptr);

public slots:
	void normalizeAll();
	void showCorrelations();
	void showLearnMfn();
};

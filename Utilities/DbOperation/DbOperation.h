#pragma once

#include <memory>
#include <QtWidgets/QMainWindow>
#include <QSqlTableModel>
#include <QTableView>
#include <QDockWidget>
#include <Shared/DbAccess/DbAccess.h>
#include <Shared/DbAccess/MetaDataModel.h>
#include "ui_DbOperation.h"
#include "SourcesModel.h"
#include "MetaDataView.h"
#include "SourcesView.h"

//----------------------------------------------------------------------------------------------------------
// class DbOperation
//----------------------------------------------------------------------------------------------------------
class DbOperation : public QMainWindow
{
	Q_OBJECT;

// data
private:
	static DbOperation*				instance_;

	Ui::DbOperationClass			ui_;

	std::unique_ptr<DbAccess>		db_;

	// metadata view
	std::unique_ptr<MetaDataModel>	metadata_model_;
	std::unique_ptr<MetaDataView>	metadata_view_;
	std::unique_ptr<QDockWidget>	sources_widget_;
	std::unique_ptr<SourcesModel>	sources_model_;
	std::unique_ptr<SourcesView>	sources_view_;

// methods
private:
	void _createSourcesView();
	void _createMetadataView();

public:
	DbOperation(QWidget* parent = Q_NULLPTR);
	~DbOperation();

	DbAccess& db() const { return *db_; }

	static DbOperation* instance() noexcept { return instance_; }
};

#include "stdafx.h"
#include "DbOperation.h"
#include <QSqlError>

//----------------------------------------------------------------------------------------------------------
DbOperation* DbOperation::instance_ = nullptr;

//----------------------------------------------------------------------------------------------------------
DbOperation::DbOperation(QWidget* parent)
	: QMainWindow(parent)
	, db_(std::make_unique<DbAccess>())
{
	assert(!instance_);
	instance_ = this;

	ui_.setupUi(this);

	_createMetadataView();
	_createSourcesView();
}
//----------------------------------------------------------------------------------------------------------
DbOperation::~DbOperation()
{
	instance_ = nullptr;
}
//----------------------------------------------------------------------------------------------------------
void DbOperation::_createMetadataView()
{
	metadata_model_ = std::make_unique<MetaDataModel>(*db_, this);

	metadata_view_ = std::make_unique<MetaDataView>(metadata_model_.get());
	metadata_view_->setModel(metadata_model_.get());

	setCentralWidget(metadata_view_.get());

	QObject::connect(ui_.actionDeleteColumn, &QAction::triggered, metadata_view_.get(), &MetaDataView::delete_column);
	QObject::connect(ui_.actionNormalizeColumn, &QAction::triggered, metadata_view_.get(), &MetaDataView::normalize_column);
	QObject::connect(ui_.actionNormalizeAll, &QAction::triggered, metadata_view_.get(), &MetaDataView::normalize_all);
	QObject::connect(ui_.actionMake_target, &QAction::triggered, metadata_view_.get(), &MetaDataView::make_target);
	QObject::connect(ui_.actionAdjustSplits, &QAction::triggered, metadata_view_.get(), &MetaDataView::adjust_splits);
}
//----------------------------------------------------------------------------------------------------------
void DbOperation::_createSourcesView()
{
	sources_widget_ = std::make_unique<QDockWidget>("Sources", this);
	sources_widget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::TopDockWidgetArea);
	addDockWidget(Qt::TopDockWidgetArea, sources_widget_.get());

	sources_model_ = std::make_unique<SourcesModel>(sources_widget_.get());
	sources_model_->load();

	sources_view_ = std::make_unique<SourcesView>(sources_model_.get(), sources_widget_.get());
	sources_view_->setRootIsDecorated(true);
	sources_view_->setItemsExpandable(true);
	sources_view_->setExpandsOnDoubleClick(true);
	sources_view_->setModel(sources_model_.get());
	
	sources_widget_->setWidget(sources_view_.get());
}

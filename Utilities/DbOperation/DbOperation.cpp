#include "stdafx.h"
#include "DbOperation.h"
#include <QSqlError>
#include <Shared/ChartLib/Candlestick.h>

//----------------------------------------------------------------------------------------------------------
DbOperation* DbOperation::instance_ = nullptr;

//----------------------------------------------------------------------------------------------------------
DbOperation::DbOperation(QWidget* parent)
	: QMainWindow{parent}
	, db_{std::make_unique<DbAccess>()}
	, operations_{operation::create_operations()}
{
	assert(!instance_);
	instance_ = this;

	ui_.setupUi(this);

	_fill_operation_menu();

	_createMetadataView();
	_createSourcesView();

	QObject::connect(ui_.actionCandlestick, &QAction::triggered, this, &DbOperation::show_candlestick);
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

	QObject::connect(ui_.actionDeleteColumn, &QAction::triggered, metadata_view_.get(), &MetaDataView::delete_checked_columns);
	QObject::connect(ui_.actionNormalizeColumn, &QAction::triggered, metadata_view_.get(), &MetaDataView::normalize_checked_columns);
	QObject::connect(ui_.actionNormalizeAll, &QAction::triggered, metadata_view_.get(), &MetaDataView::normalize_all);
	QObject::connect(ui_.actionMake_target, &QAction::triggered, metadata_view_.get(), &MetaDataView::make_target);
	QObject::connect(ui_.actionAdjustSplits, &QAction::triggered, metadata_view_.get(), &MetaDataView::adjust_splits);
	QObject::connect(ui_.action_splits_ohlcv_, &QAction::triggered, metadata_view_.get(), &MetaDataView::adjust_splits_ohlcv);
	QObject::connect(ui_.actionCreate_features, &QAction::triggered, metadata_view_.get(), &MetaDataView::create_features);
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
//----------------------------------------------------------------------------------------------------------
void DbOperation::_fill_operation_menu()
{
	auto menu{menuBar()->addMenu("&Operations")};

	for (auto& op : operations_)
	{
		unique_ptr<QAction> act{make_unique<QAction>(QString::fromStdString(op->name()), this)};
		act->setStatusTip(QString::fromStdString(op->description()));
		QObject::connect(act.get(), &QAction::triggered, this,
			[op, this]()
			{
				op->run(this);
			}
		);
		menu->addAction(act.release());
	}
}
//----------------------------------------------------------------------------------------------------------
void DbOperation::show_candlestick()
{
	unique_ptr<QDockWidget> dock{make_unique<QDockWidget>("Candlestick", this)};
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	addDockWidget(Qt::RightDockWidgetArea, dock.get());
	unique_ptr<chart::Candlestick> chart{make_unique<chart::Candlestick>()};
	if (!chart->show_load_data_dlg(*db_))
	{
		return;
	}
	unique_ptr<QtCharts::QChartView> chart_view{make_unique<QtCharts::QChartView>(chart.get())};
	chart_view->setRenderHint(QPainter::Antialiasing);
	dock->setWidget(chart_view.get());

	chart.release();
	chart_view.release();
	dock.release();
}

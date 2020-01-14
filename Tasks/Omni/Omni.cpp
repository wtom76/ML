#include "stdafx.h"
#include <QtCharts/QScatterSeries>
#include <QtCharts/QChartView>
#include "Omni.h"
#include "Correlations.h"
#include "CorrelationsDlg.h"
#include "CorrelationsModel.h"
#include "LearnMfnDlg.h"

using namespace std;

//----------------------------------------------------------------------------------------------------------
Omni::Omni(QWidget* parent)
	: QMainWindow(parent)
	, db_(std::make_shared<DbAccess>())
{
	ui_.setupUi(this);

	_createMetadataView();

	connect(ui_.action_correlations_, &QAction::triggered, this, &Omni::showCorrelations);
	connect(ui_.action_mfn, &QAction::triggered, this, &Omni::showLearnMfn);
}
//----------------------------------------------------------------------------------------------------------
void Omni::_createMetadataView()
{
	metadata_model_ = std::make_unique<MetaDataModel>(*db_, this);
	ui_.meta_data_view_->setModel(metadata_model_.get());
	for (int i = 0; i < ui_.meta_data_view_->header()->count(); ++i)
	{
		ui_.meta_data_view_->resizeColumnToContents(i);
	}
}
//----------------------------------------------------------------------------------------------------------
QtCharts::QChart* Omni::_createChart()
{
	QtCharts::QChart* chart = new QtCharts::QChart;
	chart->setTitle("Series chart");
	const auto col_metas = metadata_model_->columnInfos();
	vector<string> col_names;
	string table_name;
	for (const auto& col_info : col_metas)
	{
		if (table_name.empty())
		{
			table_name = col_info.table_;
		}
		else
		{
			assert(table_name == col_info.table_);		// different tables are not supported
		}
		col_names.emplace_back(col_info.column_);
	}
	DataFrame df = db_->load_data(g_dest_schema, table_name, col_names);
	for (const string& col_name : df.names())
	{
		QtCharts::QScatterSeries* dst_series = new QtCharts::QScatterSeries(chart);
		dst_series->setName(QString::fromStdString(col_name));
		{
			auto idx_i = df.index().cbegin();
			const DataFrame::series_t& series = *df.series(col_name);
			for (double point : series)
			{
				dst_series->append(static_cast<double>(idx_i->time_since_epoch().count()), point);
				++idx_i;
			}
			chart->addSeries(dst_series);
		}
	}
	chart->createDefaultAxes();
	return chart;
}
//----------------------------------------------------------------------------------------------------------
void Omni::_createChartView()
{
	QtCharts::QChartView* chartView = new QtCharts::QChartView(_createChart());
	ui_.gridLayout->addWidget(chartView, 1, 0, 1, 1);
}
//----------------------------------------------------------------------------------------------------------
void Omni::showCorrelations()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	const std::vector<ColumnMetaData> infos = metadata_model_->columnInfos();
	CorrelationsModel* model = new CorrelationsModel(infos, *db_, this);
	CorrelationsDlg* wnd = new CorrelationsDlg(this);
	wnd->view()->setModel(model);
	wnd->show();
	QApplication::restoreOverrideCursor();
}
//----------------------------------------------------------------------------------------------------------
void Omni::showLearnMfn()
{
	QApplication::setOverrideCursor(Qt::WaitCursor);
	LearnMfnDlg* wnd = new LearnMfnDlg(metadata_model_->columnInfos(), db_, this);
	wnd->show();
	QApplication::restoreOverrideCursor();
}

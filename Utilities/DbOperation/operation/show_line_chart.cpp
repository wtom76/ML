#include "stdafx.h"
#include "show_line_chart.h"
#include <Shared/ChartLib/line.h>
#include "../DbOperation.h"

//---------------------------------------------------------------------------------------------------------
operation::show_line_chart::~show_line_chart()
{}
//---------------------------------------------------------------------------------------------------------
std::string operation::show_line_chart::name() const
{
	return "Line chart"s;
}
//---------------------------------------------------------------------------------------------------------
std::string operation::show_line_chart::description() const
{
	return "Draw line chart for checked series"s;
}
//---------------------------------------------------------------------------------------------------------
void operation::show_line_chart::run(DbOperation* ctx)
{
	const MetaDataModel& mm{ctx->meta_model()};
	const vector<int> selected{mm.selected_columns()};
	if (selected.empty())
	{
		return;
	}
	vector<string> cols;
	const string schema{"ready"s};
	const string table{move(mm.col_meta(selected.front()).table_)};
	cols.emplace_back(move(mm.col_meta(selected.front()).column_));
	for (int i{1}; i < selected.size(); ++i)
	{
		if (table != mm.col_meta(selected[i]).table_)
		{
			continue;
		}
		cols.emplace_back(move(mm.col_meta(selected[i]).column_));
	}

	unique_ptr<QDockWidget> dock{make_unique<QDockWidget>("Line chart", ctx)};
	dock->setAllowedAreas(Qt::AllDockWidgetAreas);
	ctx->addDockWidget(Qt::RightDockWidgetArea, dock.get());
	unique_ptr<chart::line> chart{make_unique<chart::line>()};

	chart->set_columns(ctx->db(), schema, table, cols);
	unique_ptr<QtCharts::QChartView> chart_view{make_unique<QtCharts::QChartView>(chart.get())};
	chart_view->setRenderHint(QPainter::Antialiasing);
	dock->setWidget(chart_view.get());

	chart.release();
	chart_view.release();
	dock.release();
}

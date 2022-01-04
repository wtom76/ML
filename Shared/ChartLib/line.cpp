#include "stdafx.h"
#include <QLineSeries>
#include <QDateTimeAxis>
#include "line.h"
#include "line_dlg.h"
#include <Shared/DbAccess/DbAccess.h>

namespace chart
{
	//---------------------------------------------------------------------------------------------------------
	line::line()
	{
	}
	//---------------------------------------------------------------------------------------------------------
	line::~line()
	{
	}
	//---------------------------------------------------------------------------------------------------------
	bool line::show_load_data_dlg(DbAccess& db)
	{
		unique_ptr<chart::line_dlg> dlg{make_unique<chart::line_dlg>()};
		util::config::load(*dlg, util::config::file_path("line_chart"));
		if (QDialog::Accepted == dlg->exec())
		{
			util::config::store(*dlg, util::config::file_path("line_chart"));
			_set_data(db.load_data(dlg->schema(), dlg->table(), {dlg->values_col()}));
			return true;
		}
		return false;
	}
	//---------------------------------------------------------------------------------------------------------
	void line::_set_data(const DataFrame& df)
	{
		setTitle(QString::fromStdString(df.names().front()));
		setAnimationOptions(QChart::SeriesAnimations);

		unique_ptr<QtCharts::QLineSeries> series{make_unique<QtCharts::QLineSeries>()};
		series->setColor(QColor(Qt::blue));

		const DataFrame::index_series_t& index{df.index()};
		const DataFrame::series_t& values{df.series(0)};

		for (size_t i = 0; i != df.row_count(); ++i)
		{
			series->append(
				static_cast<double>(chrono::duration_cast<chrono::milliseconds>(index[i].time_since_epoch()).count()),
				values[i]);
		}

		addSeries(series.get());

		createDefaultAxes();

		{
			unique_ptr<QtCharts::QDateTimeAxis> x{make_unique<QtCharts::QDateTimeAxis>()};
			x->setFormat("yyyy-MM-dd");
			addAxis(x.get(), Qt::AlignBottom);
			series->attachAxis(x.get());
			x.release();
		}

		series.release();
	}
}

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
	void line::set_columns(DbAccess& db, const string& schema, const string& table, const vector<string>& col_names)
	{
		_set_data(db.load_data(schema, table, col_names));
	}
	//---------------------------------------------------------------------------------------------------------
	void line::_set_data(const DataFrame& df)
	{
		setTitle("Line chart");
		setAnimationOptions(QChart::SeriesAnimations);

		const DataFrame::index_series_t& index{df.index()};
		createDefaultAxes();
		unique_ptr<QtCharts::QDateTimeAxis> x{make_unique<QtCharts::QDateTimeAxis>()};
		x->setFormat("yyyy-MM-dd");
		addAxis(x.get(), Qt::AlignBottom);

		for (int series_idx{0}; series_idx < df.col_count(); ++series_idx)
		{
			unique_ptr<QtCharts::QLineSeries> series{make_unique<QtCharts::QLineSeries>()};
			series->setName(QString::fromStdString(df.names()[series_idx]));
			{
				const QColor color{static_cast<Qt::GlobalColor>(Qt::darkRed + (series_idx % (Qt::transparent - Qt::darkRed)))};
				series->setColor(color);
			}

			const DataFrame::series_t& values{df.series(series_idx)};

			for (size_t i = 0; i < df.row_count(); ++i)
			{
				series->append(
					static_cast<double>(chrono::duration_cast<chrono::milliseconds>(index[i].time_since_epoch()).count()),
					values[i]);
			}

			addSeries(series.get());
			series->attachAxis(x.get());
			series.release();
		}
		x.release();
	}
}

#include "stdafx.h"
#include <QCandlestickSeries>
#include <QCandlestickSet>
#include <QDateTimeAxis>
#include "Candlestick.h"
#include "CandlestickDlg.h"
#include <Shared/DbAccess/DbAccess.h>

namespace chart
{
	//---------------------------------------------------------------------------------------------------------
	Candlestick::Candlestick()
	{
	}
	//---------------------------------------------------------------------------------------------------------
	Candlestick::~Candlestick()
	{
	}
	//---------------------------------------------------------------------------------------------------------
	bool Candlestick::show_load_data_dlg(DbAccess& db)
	{
		unique_ptr<chart::CandlestickDlg> dlg{make_unique<chart::CandlestickDlg>()};
		util::config::load(*dlg, util::config::file_path("chart"));
		if (QDialog::Accepted == dlg->exec())
		{
			util::config::store(*dlg, util::config::file_path("chart"));
			_load_data(dlg->schema(), dlg->table(), {dlg->open_col(), dlg->high_col(), dlg->low_col(), dlg->close_col()}, db);
			return true;
		}
		return false;
	}
	//---------------------------------------------------------------------------------------------------------
	void Candlestick::_load_data(const std::string& schema, const std::string& table_name, const std::vector<std::string>& col_names, DbAccess& db)
	{
		_set_data(db.load_data(schema, table_name, col_names));
	}
	//---------------------------------------------------------------------------------------------------------
	void Candlestick::_set_data(const DataFrame& df)
	{
		setTitle("Title");
		setAnimationOptions(QChart::SeriesAnimations);

		unique_ptr<QtCharts::QCandlestickSeries> series{make_unique<QtCharts::QCandlestickSeries>()};
		series->setIncreasingColor(QColor(Qt::green));
		series->setDecreasingColor(QColor(Qt::red));

		const DataFrame::index_series_t& index{df.index()};
		const DataFrame::series_t& open{df.series("open"s)};
		const DataFrame::series_t& high{df.series("high"s)};
		const DataFrame::series_t& low{df.series("low"s)};
		const DataFrame::series_t& close{df.series("close"s)};

		for (size_t i = 0; i != df.row_count(); ++i)
		{
			QtCharts::QCandlestickSet* ds{new QtCharts::QCandlestickSet{
				open[i], high[i], low[i], close[i],
				static_cast<double>(chrono::duration_cast<chrono::milliseconds>(index[i].time_since_epoch()).count())
			}};
			series->append(ds);
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

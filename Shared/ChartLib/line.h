#pragma once

#include <QChart>
#include <QChartView>

class DbAccess;

namespace chart
{
	//---------------------------------------------------------------------------------------------------------
	/// class Candlestick
	//---------------------------------------------------------------------------------------------------------
	class line : public QtCharts::QChart
	{
		Q_OBJECT;

	private:
		void _set_data(const DataFrame& df);

	public:
		line();
		~line();

		bool show_load_data_dlg(DbAccess& db);
	};
}

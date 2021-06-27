#pragma once

#include <QChart>
#include <QChartView>

class DbAccess;

namespace chart
{
	//---------------------------------------------------------------------------------------------------------
	/// class Candlestick
	//---------------------------------------------------------------------------------------------------------
	class Candlestick : public QtCharts::QChart
	{
		Q_OBJECT;

	private:
		void _set_data(const DataFrame& df);

	public:
		Candlestick();
		~Candlestick();

		bool show_load_data_dlg(DbAccess& db);
	};
}

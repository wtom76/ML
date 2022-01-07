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

		void set_columns(DbAccess& db, const string& schema, const string& table, const vector<string>& col_names);
	};
}

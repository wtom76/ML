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
		void _load_data(const std::string& schema, const std::string& table_name, const std::vector<std::string>& col_names, DbAccess& db);
		void _set_data(const DataFrame& df);

	public:
		Candlestick();
		~Candlestick();

		bool show_load_data_dlg(DbAccess& db);
	};
}

#include "stdafx.h"
#include "DbAccess.h"
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QVariantList>

//----------------------------------------------------------------------------------------------------------
DbAccess::DbAccess()
{
	QSqlDatabase db_ = QSqlDatabase::addDatabase("QPSQL");
	db_.setHostName("localhost");
	db_.setDatabaseName("ML");
	db_.setUserName("ml_user");
	db_.setPassword("ml_user");
	if (!db_.open())
	{
		throw std::runtime_error(std::string("Failed to connect to database. ") +
			db_.lastError().text().toStdString());
	}
}
//----------------------------------------------------------------------------------------------------------
DbAccess::~DbAccess()
{
}
//----------------------------------------------------------------------------------------------------------
bool DbAccess::load(wtom::ml::Data::Frame<double>& frame)
{
	constexpr __int64 col_count = 7;
	frame.clear();
	QSqlQuery query(
		"SELECT borrower_funds, gdp_1, gdp_pct_prev_1, consumer_price_index_pct_prev_1, gdp_2, gdp_pct_prev_2, consumer_price_index_pct_prev_2 "
		"FROM ready.mortgage_loan_prepayments;"
	);
	frame.setColumns(col_count);
	if (query.size() > 0)
	{
		frame.reserve(query.size());
	}
	std::vector<double> row(col_count, 0.);
	while (query.next())
	{
		for (__int64 i = 0; i < col_count; ++i)
		{
			row[i] = query.value(i).toDouble();
		}
		frame.push_back(row);
	}
	if (query.lastError().isValid())
	{
		//log()->error("Failed to load stored calculations. {:}", query.lastError().text().toStdString());
		return false;
	}
	return true;
}

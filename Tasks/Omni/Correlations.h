#pragma once

#include <vector>
#include <Shared/DbAccess/ColumnData.h>

//----------------------------------------------------------------------------------------------------------
// class Correlations
//----------------------------------------------------------------------------------------------------------
class Correlations
{
private:
	using Matrix = std::vector<double>;

private:
	Matrix matrix_;

private:
	void _loadCalc(const std::vector<ColumnData>& data);

public:
	explicit Correlations(const std::vector<ColumnData>& data);
	~Correlations();
};


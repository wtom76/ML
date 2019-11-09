#pragma once

#include <string>
#include <vector>
#include <Shared/DbAccess/DataFrame.h>

//----------------------------------------------------------------------------------------------------------
// class Correlations
//----------------------------------------------------------------------------------------------------------
class Correlations
{
private:
	using Matrix = std::vector<double>;

private:
	Matrix matrix_;
	size_t dimention_ = 0;

public:
	explicit Correlations();
	~Correlations();

	void calculate(const std::vector<std::string>& col_names, const DataFrame& data);
	size_t dimention() const noexcept { return dimention_; }
	double value(size_t row, size_t col) const noexcept { return matrix_[row * dimention_ + col]; }
};


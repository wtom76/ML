#pragma once
#include <Shared/Data/Frame.hpp>

class DbAccess
{
public:
	DbAccess();
	~DbAccess();

	bool load(wtom::ml::Data::Frame<double>& frame);
};

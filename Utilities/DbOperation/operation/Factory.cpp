#include "stdafx.h"
#include "Factory.h"
#include "UpdateDates.h"
#include "show_line_chart.h"

//---------------------------------------------------------------------------------------------------------
operation::Container operation::create_operations()
{
	Container ops{};

	ops.reserve(2);

	ops.emplace_back(make_shared<UpdateDates>());
	ops.emplace_back(make_shared<show_line_chart>());

	return ops;
}

#include "stdafx.h"
#include "Factory.h"
#include "UpdateDates.h"

//---------------------------------------------------------------------------------------------------------
operation::Container operation::create_operations()
{
	Container ops{};

	ops.reserve(1);

	ops.emplace_back(make_shared<UpdateDates>());

	return ops;
}

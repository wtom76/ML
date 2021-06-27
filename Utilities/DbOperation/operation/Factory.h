#pragma once

#include "Interface.h"

namespace operation
{
	using Container = std::vector<std::shared_ptr<operation::Interface>>;

	Container create_operations();
}
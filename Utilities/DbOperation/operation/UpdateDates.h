#pragma once

#include "Interface.h"

namespace operation
{
	class UpdateDates : public Interface
	{
		bool _date_boundary(const DataFrame& data, pair<system_clock::time_point, system_clock::time_point>& min_max);
	public:
		~UpdateDates() override;
		std::string name() const override;
		std::string description() const override;
		void run(DbOperation* ctx) override;
	};
}
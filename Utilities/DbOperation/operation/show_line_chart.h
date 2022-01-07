#pragma once

#include "Interface.h"

namespace operation
{
	class show_line_chart : public Interface
	{
	public:
		~show_line_chart() override;
		std::string name() const override;
		std::string description() const override;
		void run(DbOperation* ctx) override;
	};
}
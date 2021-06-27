#pragma once
#include <string>

class DbOperation;

namespace operation
{
	class Interface
	{
	public:
		virtual ~Interface(){}
		virtual std::string name() const = 0;
		virtual std::string description() const = 0;
		virtual void run(DbOperation* model) = 0;
	};
}
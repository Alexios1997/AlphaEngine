#pragma once
#include <string>

namespace AlphaEngine 
{
	class Logger {
	public:
		static void Log(const std::string& message);
		static void Err(const std::string& message);
	};

}
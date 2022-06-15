#pragma once
#include <vector>
#include <string>
#include <string_view>

namespace str_tool
{
	std::vector<std::string> split(const std::string& str, std::string_view delimiter);
	size_t replace_all(std::string& inout, std::string_view what, std::string_view with);
}

#pragma once
#include <vector>
#include <string>
#include <string_view>

using namespace std;

namespace str_tool
{
	vector<string> split(const string& str, string_view delimiter);
	size_t replace_all(string& inout, string_view what, string_view with);
}

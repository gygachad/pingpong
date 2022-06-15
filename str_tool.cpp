#include "str_tool.h" 

namespace str_tool
{
	// ("",  '.') -> [""]
	// ("11", '.') -> ["11"]
	// ("..", '.') -> ["", "", ""]
	// ("11.", '.') -> ["11", ""]
	// (".11", '.') -> ["", "11"]
	// ("11.22", '.') -> ["11", "22"]
	std::vector<std::string> split(const std::string& str, std::string_view delimiter)
	{		
		std::string str_copy = str;

		std::vector<std::string> result;

		size_t pos = 0;
		std::string token;
		while ((pos = str_copy.find(delimiter)) != std::string::npos)
		{
			token = str_copy.substr(0, pos);
			result.push_back(token);
			str_copy.erase(0, pos + delimiter.length());
		}

		if(!str_copy.empty())
			result.push_back(str_copy);

		return result;
	}

	size_t replace_all(std::string& inout, std::string_view what, std::string_view with)
	{
		size_t count{};
		for (std::string::size_type pos{};
			inout.npos != (pos = inout.find(what.data(), pos, what.length()));
			pos += with.length(), ++count)
		{
			inout.replace(pos, what.length(), with.data(), with.length());
		}
		return count;
	}
}
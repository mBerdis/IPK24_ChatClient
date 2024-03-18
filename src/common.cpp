#include "common.h"
#include <algorithm>
#include <iostream>

void print_err(const std::string message)
{
	std::cerr << "ERR: " << message << "\n" << std::flush;
}

void print_msg(const std::string displayName, const std::string message)
{
	std::cout << displayName << ": " << message << "\n" << std::flush;
}

/*
* Compares in case-insensitive manner if given string equals given keyword.
*/
bool eq_kw(const std::string str, const std::string kw)
{
	return to_lower(str) == to_lower(kw);
}

std::string to_lower(std::string data)
{
	std::transform(data.begin(), data.end(), data.begin(),
		[](unsigned char c) { return std::tolower(c); });
	return data;
}

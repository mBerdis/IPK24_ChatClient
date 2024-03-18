#include "common.h"
#include <iostream>

void print_err(const std::string message)
{
	std::cerr << "ERR: " << message << "\n" << std::flush;
}

void print_msg(const std::string displayName, const std::string message)
{
	std::cout << displayName << ": " << message << "\n" << std::flush;
}

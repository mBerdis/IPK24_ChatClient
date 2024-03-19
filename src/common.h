#pragma once
#include <string>

// exit codes
constexpr auto ERR_CONNECTION	= 50;
constexpr auto ERR_SERVER		= 40;
constexpr auto SUCCESS			= 0;
//			   EXIT_FAILURE		= 1;

void print_err(const std::string message);
void print_msg(const std::string displayName, const std::string message);
bool eq_kw(const std::string str, const std::string kw);
std::string to_lower(std::string data);
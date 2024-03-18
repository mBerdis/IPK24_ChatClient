#pragma once
#include <string>

void print_err(const std::string message);
void print_msg(const std::string displayName, const std::string message);
bool eq_kw(const std::string str, const std::string kw);
std::string to_lower(std::string data);
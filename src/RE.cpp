#include "RE.hpp"
#include <cstring>

RuntimeError::RuntimeError(std::string s1) : s(s1) {}
std::string RuntimeError::message() const { return s; }
//const char* RuntimeError::what() const noexcept{
   //return s.c_str();
//}
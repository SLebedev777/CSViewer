#pragma once

#include <string>

size_t Utf8StrLen(const std::string& utf8_str); // returns length of Utf8 encoded string in Unicode symbols (not bytes!)
std::string Utf8SubStr(const std::string& utf8_str, size_t pos, size_t count);  // pos and count are in Unicode symbols (not bytes!)
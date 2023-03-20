#include "UTF8Utils.h"

size_t Utf8StrLen(const std::string& utf8_str)
{
	size_t len = 0;
	const char* p = utf8_str.c_str();
	while (*p)
	{
		if ((*p++ & 0xc0) != 0x80)  // count every first octet (beginning of the block) - it never starts with bits 10
			++len;
	}
	return len;
}

std::string Utf8SubStr(const std::string& utf8_str, size_t pos, size_t count)
{
	size_t len = 0;
	size_t pos_in_bytes_start = 0;
	size_t pos_in_bytes_end = 0;  // end position of target substring in bytes
	const char* p = utf8_str.c_str();

	while (*p)
	{
		if ((*p++ & 0xc0) != 0x80)  // count every first octet (beginning of the block) - it never starts with bits 10
			++len;
		if (len <= pos)
			++pos_in_bytes_start;
		if (len <= pos + count)
			++pos_in_bytes_end;
	}
	return utf8_str.substr(pos_in_bytes_start, pos_in_bytes_end - pos_in_bytes_start);
}
#pragma once

#include <string>
#include <libiconv/include/iconv.h>

namespace utils
{
	class IConvConverter
	{
	public:

		enum class EncodingErrorMode
		{
			STRICT = 0,
			IGNORE,
			REPLACE
		};

		IConvConverter(const std::string& input_encoding, const std::string& output_encoding);
		~IConvConverter();
		
		std::string convert(const std::string& str, EncodingErrorMode error_mode = EncodingErrorMode::REPLACE);
		void setReplaceSymbol(char c) { m_replaceSymbol = c; }

	private:
		iconv_t m_conv = nullptr;
		char m_replaceSymbol = '?';
	};
}
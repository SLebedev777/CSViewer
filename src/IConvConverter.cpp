#include "IConvConverter.h"
#include <stdexcept>
#include <vector>

namespace utils
{
	IConvConverter::IConvConverter(const std::string& input_encoding, const std::string& output_encoding)
	{
        m_conv = ::iconv_open(output_encoding.c_str(), input_encoding.c_str());
        if (m_conv == (iconv_t)-1)
        {
            throw std::runtime_error("libiconv init error");
        }
	}

    IConvConverter::~IConvConverter()
    {
        if (m_conv)
            iconv_close(m_conv);
    }

    std::string IConvConverter::convert(const std::string& str, EncodingErrorMode error_mode)
    {
        size_t BUFSIZE = 1024;
        std::vector<char> buf(BUFSIZE); // future result
        char* dst_ptr = &buf[0];  // should NOT be const, because libiconv will write through it
        size_t dst_size = BUFSIZE; // here libiconv will write actual size of converted result string
        const char* src_ptr = str.data();
        size_t SRCSIZE = str.size();
        size_t res = ::iconv(m_conv, &src_ptr, &SRCSIZE, &dst_ptr, &dst_size);
        std::string result;
        result.append(&buf[0], BUFSIZE - dst_size);
        return result;
    }
}
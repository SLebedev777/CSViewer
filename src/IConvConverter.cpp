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
        std::string result; // future result
        size_t BUFSIZE = 1024;
        std::vector<char> buf(BUFSIZE);  // temp buffer
        const char* src_ptr = str.data();
        size_t src_size = str.size();

        while (src_size > 0)
        {
            char* dst_ptr = &buf[0];  // should NOT be const, because libiconv will write through it
            size_t dst_size = BUFSIZE; // here libiconv will write actual size of converted result string
            size_t res = ::iconv(m_conv, &src_ptr, &src_size, &dst_ptr, &dst_size);
            result.append(&buf[0], BUFSIZE - dst_size);  // добавляем в результат всё, что удалось наконвертировать
            
            if (res == (size_t)-1)
            {
                switch (errno)
                {
                case EILSEQ:
                case EINVAL:
                    // found invalid symbol
                    if (error_mode == EncodingErrorMode::STRICT)
                    {
                        throw std::runtime_error("iconv conversion error: failed to convert symbol");
                    }
                    // Skip 1 byte in src sequence and try to convert the remaining
                    ++src_ptr;
                    --src_size;
                    // insert "?" in dst instead of invalid symbol
                    if (error_mode == EncodingErrorMode::REPLACE)
                        result += m_replaceSymbol;
                    break;
                case E2BIG:
                    // not enough size of buffer. Do nothing, just try to convert the remaining.
                    break;
                default:
                    throw std::runtime_error("iconv conversion error: unknown error");
                }
            }

        }
        return result;
    }
}
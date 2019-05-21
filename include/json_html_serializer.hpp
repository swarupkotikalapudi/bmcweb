#include <nlohmann/json.hpp>

#include <algorithm>

namespace json_html_util
{

static constexpr uint8_t utf8Accept = 0;
static constexpr uint8_t utf8Reject = 1;

inline uint8_t decode(uint8_t& state, uint32_t& codePoint,
                      const uint8_t byte) noexcept
{
    // clang-format off
    static const std::array<std::uint8_t, 400> utf8d =
    {
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 00..1F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 20..3F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 40..5F
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 60..7F
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, // 80..9F
            7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, // A0..BF
            8, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // C0..DF
            0xA, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x4, 0x3, 0x3, // E0..EF
            0xB, 0x6, 0x6, 0x6, 0x5, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, // F0..FF
            0x0, 0x1, 0x2, 0x3, 0x5, 0x8, 0x7, 0x1, 0x1, 0x1, 0x4, 0x6, 0x1, 0x1, 0x1, 0x1, // s0..s0
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, // s1..s2
            1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, // s3..s4
            1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1, // s5..s6
            1, 3, 1, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 // s7..s8
        }
    };
    // clang-format on

    if (state > 0x8)
    {
        return state;
    }

    const uint8_t type = utf8d[byte];

    codePoint = (state != utf8Accept)
                    ? (byte & 0x3fu) | (codePoint << 6)
                    : static_cast<uint32_t>(0xff >> type) & (byte);

    state = utf8d[256u + state * 16u + type];
    return state;
}

inline void dump_escaped(std::string& out, const std::string& str)
{
    std::array<char, 512> stringBuffer{{}};
    uint32_t codePoint = 0;
    uint8_t state = utf8Accept;
    std::size_t bytes = 0; // number of bytes written to string_buffer

    // number of bytes written at the point of the last valid byte
    std::size_t bytes_after_last_accept = 0;
    std::size_t undumped_chars = 0;

    for (std::size_t i = 0; i < str.size(); ++i)
    {
        const uint8_t byte = static_cast<uint8_t>(str[i]);

        switch (decode(state, codePoint, byte))
        {
            case utf8Accept: // decode found a new code point
            {
                switch (codePoint)
                {
                    case 0x08: // backspace
                    {
                        stringBuffer[bytes++] = '\\';
                        stringBuffer[bytes++] = 'b';
                        break;
                    }

                    case 0x09: // horizontal tab
                    {
                        stringBuffer[bytes++] = '\\';
                        stringBuffer[bytes++] = 't';
                        break;
                    }

                    case 0x0A: // newline
                    {
                        stringBuffer[bytes++] = '\\';
                        stringBuffer[bytes++] = 'n';
                        break;
                    }

                    case 0x0C: // formfeed
                    {
                        stringBuffer[bytes++] = '\\';
                        stringBuffer[bytes++] = 'f';
                        break;
                    }

                    case 0x0D: // carriage return
                    {
                        stringBuffer[bytes++] = '\\';
                        stringBuffer[bytes++] = 'r';
                        break;
                    }

                    case 0x22: // quotation mark
                    {
                        stringBuffer[bytes++] = '&';
                        stringBuffer[bytes++] = 'q';
                        stringBuffer[bytes++] = 'u';
                        stringBuffer[bytes++] = 'o';
                        stringBuffer[bytes++] = 't';
                        stringBuffer[bytes++] = ';';
                        break;
                    }

                    case 0x27: // apostrophe
                    {
                        stringBuffer[bytes++] = '&';
                        stringBuffer[bytes++] = 'a';
                        stringBuffer[bytes++] = 'p';
                        stringBuffer[bytes++] = 'o';
                        stringBuffer[bytes++] = 's';
                        stringBuffer[bytes++] = ';';
                        break;
                    }

                    case 0x26: // ampersand
                    {
                        stringBuffer[bytes++] = '&';
                        stringBuffer[bytes++] = 'a';
                        stringBuffer[bytes++] = 'm';
                        stringBuffer[bytes++] = 'p';
                        stringBuffer[bytes++] = ';';
                        break;
                    }

                    case 0x3C: // less than
                    {
                        stringBuffer[bytes++] = '\\';
                        stringBuffer[bytes++] = 'l';
                        stringBuffer[bytes++] = 't';
                        stringBuffer[bytes++] = ';';
                        break;
                    }

                    case 0x3E: // greater than
                    {
                        stringBuffer[bytes++] = '\\';
                        stringBuffer[bytes++] = 'g';
                        stringBuffer[bytes++] = 't';
                        stringBuffer[bytes++] = ';';
                        break;
                    }

                    default:
                    {
                        // escape control characters (0x00..0x1F)
                        if ((codePoint <= 0x1F) or (codePoint >= 0x7F))
                        {
                            if (codePoint <= 0xFFFF)
                            {
                                (std::snprintf)(
                                    stringBuffer.data() + bytes, 7, "\\u%04x",
                                    static_cast<uint16_t>(codePoint));
                                bytes += 6;
                            }
                            else
                            {
                                (std::snprintf)(
                                    stringBuffer.data() + bytes, 13,
                                    "\\u%04x\\u%04x",
                                    static_cast<uint16_t>(0xD7C0 +
                                                          (codePoint >> 10)),
                                    static_cast<uint16_t>(0xDC00 +
                                                          (codePoint & 0x3FF)));
                                bytes += 12;
                            }
                        }
                        else
                        {
                            // copy byte to buffer (all previous bytes
                            // been copied have in default case above)
                            stringBuffer[bytes++] = str[i];
                        }
                        break;
                    }
                }

                // write buffer and reset index; there must be 13 bytes
                // left, as this is the maximal number of bytes to be
                // written ("\uxxxx\uxxxx\0") for one code point
                if (stringBuffer.size() - bytes < 13)
                {
                    out.append(stringBuffer.data(), bytes);
                    bytes = 0;
                }

                // remember the byte position of this accept
                bytes_after_last_accept = bytes;
                undumped_chars = 0;
                break;
            }

            case utf8Reject: // decode found invalid UTF-8 byte
            {
                // in case we saw this character the first time, we
                // would like to read it again, because the byte
                // may be OK for itself, but just not OK for the
                // previous sequence
                if (undumped_chars > 0)
                {
                    --i;
                }

                // reset length buffer to the last accepted index;
                // thus removing/ignoring the invalid characters
                bytes = bytes_after_last_accept;

                stringBuffer[bytes++] = '\\';
                stringBuffer[bytes++] = 'u';
                stringBuffer[bytes++] = 'f';
                stringBuffer[bytes++] = 'f';
                stringBuffer[bytes++] = 'f';
                stringBuffer[bytes++] = 'd';

                bytes_after_last_accept = bytes;

                undumped_chars = 0;

                // continue processing the string
                state = utf8Accept;
                break;

                break;
            }

            default: // decode found yet incomplete multi-byte code point
            {
                ++undumped_chars;
                break;
            }
        }
    }

    // we finished processing the string
    if (state == utf8Accept)
    {
        // write buffer
        if (bytes > 0)
        {
            out.append(stringBuffer.data(), bytes);
        }
    }
    else
    {
        // write all accepted bytes
        out.append(stringBuffer.data(), bytes_after_last_accept);
        out += "\\ufffd";
    }
}

inline unsigned int count_digits(uint64_t number) noexcept
{
    unsigned int n_digits = 1;
    for (;;)
    {
        if (number < 10)
        {
            return n_digits;
        }
        if (number < 100)
        {
            return n_digits + 1;
        }
        if (number < 1000)
        {
            return n_digits + 2;
        }
        if (number < 10000)
        {
            return n_digits + 3;
        }
        number = number / 10000u;
        n_digits += 4;
    }
}

template <typename NumberType,
          std::enable_if_t<std::is_same<NumberType, uint64_t>::value or
                               std::is_same<NumberType, int64_t>::value,
                           int> = 0>
void dump_integer(std::string& out, NumberType number)
{
    std::array<char, 64> number_buffer{{}};

    static constexpr std::array<std::array<char, 2>, 100> digits_to_99{{
        {'0', '0'}, {'0', '1'}, {'0', '2'}, {'0', '3'}, {'0', '4'}, {'0', '5'},
        {'0', '6'}, {'0', '7'}, {'0', '8'}, {'0', '9'}, {'1', '0'}, {'1', '1'},
        {'1', '2'}, {'1', '3'}, {'1', '4'}, {'1', '5'}, {'1', '6'}, {'1', '7'},
        {'1', '8'}, {'1', '9'}, {'2', '0'}, {'2', '1'}, {'2', '2'}, {'2', '3'},
        {'2', '4'}, {'2', '5'}, {'2', '6'}, {'2', '7'}, {'2', '8'}, {'2', '9'},
        {'3', '0'}, {'3', '1'}, {'3', '2'}, {'3', '3'}, {'3', '4'}, {'3', '5'},
        {'3', '6'}, {'3', '7'}, {'3', '8'}, {'3', '9'}, {'4', '0'}, {'4', '1'},
        {'4', '2'}, {'4', '3'}, {'4', '4'}, {'4', '5'}, {'4', '6'}, {'4', '7'},
        {'4', '8'}, {'4', '9'}, {'5', '0'}, {'5', '1'}, {'5', '2'}, {'5', '3'},
        {'5', '4'}, {'5', '5'}, {'5', '6'}, {'5', '7'}, {'5', '8'}, {'5', '9'},
        {'6', '0'}, {'6', '1'}, {'6', '2'}, {'6', '3'}, {'6', '4'}, {'6', '5'},
        {'6', '6'}, {'6', '7'}, {'6', '8'}, {'6', '9'}, {'7', '0'}, {'7', '1'},
        {'7', '2'}, {'7', '3'}, {'7', '4'}, {'7', '5'}, {'7', '6'}, {'7', '7'},
        {'7', '8'}, {'7', '9'}, {'8', '0'}, {'8', '1'}, {'8', '2'}, {'8', '3'},
        {'8', '4'}, {'8', '5'}, {'8', '6'}, {'8', '7'}, {'8', '8'}, {'8', '9'},
        {'9', '0'}, {'9', '1'}, {'9', '2'}, {'9', '3'}, {'9', '4'}, {'9', '5'},
        {'9', '6'}, {'9', '7'}, {'9', '8'}, {'9', '9'},
    }};

    // special case for "0"
    if (number == 0)
    {
        out += '0';
        return;
    }

    // use a pointer to fill the buffer
    auto buffer_ptr = begin(number_buffer);

    const bool is_negative = std::is_same<NumberType, int64_t>::value &&
                             !(number >= 0); // see issue #755
    uint64_t abs_value;

    unsigned int n_chars;

    if (is_negative)
    {
        *buffer_ptr = '-';
        abs_value = static_cast<uint64_t>(0 - number);

        // account one more byte for the minus sign
        n_chars = 1 + count_digits(abs_value);
    }
    else
    {
        abs_value = static_cast<uint64_t>(number);
        n_chars = count_digits(abs_value);
    }

    // spare 1 byte for '\0'
    if (n_chars >= number_buffer.size() - 1)
    {
        return;
    }

    // jump to the end to generate the string from backward
    // so we later avoid reversing the result
    buffer_ptr += n_chars;

    // Fast int2ascii implementation inspired by "Fastware" talk by Andrei
    // Alexandrescu See: https://www.youtube.com/watch?v=o4-CwDo2zpg
    while (abs_value >= 100)
    {
        const auto digits_index = static_cast<unsigned>((abs_value % 100));
        abs_value /= 100;
        *(--buffer_ptr) = digits_to_99[digits_index][1];
        *(--buffer_ptr) = digits_to_99[digits_index][0];
    }

    if (abs_value >= 10)
    {
        const auto digits_index = static_cast<unsigned>(abs_value);
        *(--buffer_ptr) = digits_to_99[digits_index][1];
        *(--buffer_ptr) = digits_to_99[digits_index][0];
    }
    else
    {
        *(--buffer_ptr) = static_cast<char>('0' + abs_value);
    }

    out.append(number_buffer.data(), n_chars);
}

inline void dump_float(std::string& out, double number,
                       std::true_type /*is_ieee_single_or_double*/)
{
    std::array<char, 64> number_buffer{{}};
    char* begin = number_buffer.data();
    ::nlohmann::detail::to_chars(begin, begin + number_buffer.size(), number);

    out += begin;
}

inline void dump_float(std::string& out, double number,
                       std::false_type /*is_ieee_single_or_double*/)
{
    std::array<char, 64> number_buffer{{}};
    // get number of digits for a float -> text -> float round-trip
    static constexpr auto d = std::numeric_limits<double>::max_digits10;

    // the actual conversion
    std::ptrdiff_t len = (std::snprintf)(
        number_buffer.data(), number_buffer.size(), "%.*g", d, number);

    // negative value indicates an error
    if (len <= 0)
    {
        return;
    }

    // check if buffer was large enough
    if (number_buffer.size() < static_cast<std::size_t>(len))
    {
        return;
    }

    const auto end =
        std::remove(number_buffer.begin(), number_buffer.begin() + len, ',');
    std::fill(end, number_buffer.end(), '\0');

    if ((end - number_buffer.begin()) > len)
    {
        return;
    }
    len = (end - number_buffer.begin());

    out.append(number_buffer.data(), static_cast<std::size_t>(len));

    // determine if need to append ".0"
    const bool value_is_int_like =
        std::none_of(number_buffer.begin(), number_buffer.begin() + len + 1,
                     [](char c) { return (c == '.' or c == 'e'); });

    if (value_is_int_like)
    {
        out += ".0";
    }
}

inline void dump_float(std::string& out, double number)
{
    // NaN / inf
    if (!std::isfinite(number))
    {
        out += "null";
        return;
    }

    // If float is an IEEE-754 single or double precision number,
    // use the Grisu2 algorithm to produce short numbers which are
    // guaranteed to round-trip, using strtof and strtod, resp.
    //
    // NB: The test below works if <long double> == <double>.
    static constexpr bool is_ieee_single_or_double =
        (std::numeric_limits<double>::is_iec559 and
         std::numeric_limits<double>::digits == 24 and
         std::numeric_limits<double>::max_exponent == 128) or
        (std::numeric_limits<double>::is_iec559 and
         std::numeric_limits<double>::digits == 53 and
         std::numeric_limits<double>::max_exponent == 1024);

    dump_float(out, number,
               std::integral_constant<bool, is_ieee_single_or_double>());
}

inline void dump(std::string& out, const nlohmann::json& val)
{
    switch (val.type())
    {
        case nlohmann::json::value_t::object:
        {
            if (val.empty())
            {
                out += "{}";
                return;
            }

            out += "{";

            out += "<div class=tab>";
            for (auto i = val.begin(); i != val.end();)
            {
                out += "&quot";
                dump_escaped(out, i.key());
                out += "&quot: ";

                bool inATag = false;
                if (i.key() == "@odata.id" || i.key() == "@odata.context" ||
                    i.key() == "Members@odata.nextLink" || i.key() == "Uri")
                {
                    inATag = true;
                    out += "<a href=\"";
                    dump_escaped(out, i.value());
                    out += "\">";
                }
                dump(out, i.value());
                if (inATag)
                {
                    out += "</a>";
                }
                i++;
                if (i != val.end())
                {
                    out += ",";
                }
                out += "<br>";
            }
            out += "</div>";
            out += '}';

            return;
        }

        case nlohmann::json::value_t::array:
        {
            if (val.empty())
            {
                out += "[]";
                return;
            }

            out += "[";

            out += "<div class=tab>";

            // first n-1 elements
            for (auto i = val.cbegin(); i != val.cend() - 1; ++i)
            {
                dump(out, *i);
                out += ",<br>";
            }

            // last element
            dump(out, val.back());

            out += "</div>";
            out += ']';

            return;
        }

        case nlohmann::json::value_t::string:
        {
            out += '\"';
            const std::string* ptr = val.get_ptr<const std::string*>();
            dump_escaped(out, *ptr);
            out += '\"';
            return;
        }

        case nlohmann::json::value_t::boolean:
        {
            if (*(val.get_ptr<const bool*>()))
            {
                out += "true";
            }
            else
            {
                out += "false";
            }
            return;
        }

        case nlohmann::json::value_t::number_integer:
        {
            dump_integer(out, *(val.get_ptr<const int64_t*>()));
            return;
        }

        case nlohmann::json::value_t::number_unsigned:
        {
            dump_integer(out, *(val.get_ptr<const uint64_t*>()));
            return;
        }

        case nlohmann::json::value_t::number_float:
        {
            dump_float(out, *(val.get_ptr<const double*>()));
            return;
        }

        case nlohmann::json::value_t::discarded:
        {
            out += "<discarded>";
            return;
        }

        case nlohmann::json::value_t::null:
        {
            out += "null";
            return;
        }
    }
}

inline void dump_html(std::string& out, const nlohmann::json& json)
{
    out += "<html>\n"
           "<head>\n"
           "<title>Redfish API</title>\n"
           "<link href=\"/redfish.css\" rel=\"stylesheet\">\n"
           "</head>\n"
           "<body>\n"
           "<div class=\"container\">\n"
           "<img src=\"/DMTF_Redfish_logo_2017.svg\" alt=\"redfish\" "
           "height=\"406px\" "
           "width=\"576px\">\n"
           "<div class=\"content\">\n";
    dump(out, json);
    out += "</div>\n"
           "</div>\n"
           "</body>\n"
           "</html>\n";
}

} // namespace json_html_util

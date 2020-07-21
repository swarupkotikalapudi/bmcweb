#pragma once

#include <boost/algorithm/string/predicate.hpp>
#include <boost/beast/http/fields.hpp>
#include <http_request.hpp>

#include <string>
#include <string_view>

enum class ParserError
{
    ERROR_BOUNDARY_FORMAT = 500,
    ERROR_BOUNDARY_CR,
    ERROR_BOUNDARY_LF,
    ERROR_BOUNDARY_DATA,
    ERROR_EMPTY_HEADER,
    ERROR_HEADER_NAME,
    ERROR_HEADER_VALUE,
    ERROR_HEADER_ENDING
};

enum class State
{
    ERROR,
    START,
    START_BOUNDARY,
    HEADER_FIELD_START,
    HEADER_FIELD,
    HEADER_VALUE_START,
    HEADER_VALUE,
    HEADER_VALUE_ALMOST_DONE,
    HEADERS_ALMOST_DONE,
    PART_DATA_START,
    PART_DATA,
    PART_END,
    END
};

struct FormPart
{
    boost::beast::http::fields fields;
    std::string content;
};

struct ParserErrCategory : std::error_category
{
    const char* name() const noexcept override
    {
        return "multipart parser";
    }
    std::string message(int ev) const override;
};

std::string ParserErrCategory::message(int ev) const
{
    switch (static_cast<ParserError>(ev))
    {
        case ParserError::ERROR_BOUNDARY_FORMAT:
        {
            return "Malformed. Parser boundary format error.";
        }
        case ParserError::ERROR_BOUNDARY_CR:
        {
            return "Malformed. Expected cr after boundary.";
        }
        case ParserError::ERROR_BOUNDARY_LF:
        {
            return "Malformed. Expected lf after boundary cr.";
        }
        case ParserError::ERROR_BOUNDARY_DATA:
        {
            return "Malformed. Found different boundary data than the given "
                   "one.";
        }
        case ParserError::ERROR_EMPTY_HEADER:
        {
            return "Malformed first header name character.";
        }
        case ParserError::ERROR_HEADER_NAME:
        {
            return "Malformed header name.";
        }
        case ParserError::ERROR_HEADER_VALUE:
        {
            return "Malformed header value: lf expected after cr";
        }
        case ParserError::ERROR_HEADER_ENDING:
        {
            return "Malformed header ending: lf expected after cr";
        }
        default:
        {
            return "unrecognized error";
        }
    }
}

const ParserErrCategory parserErrCategory{};

class MultipartParser
{
  public:
    MultipartParser() = default;

    [[nodiscard]] std::error_code parse(const crow::Request& req)
    {
        std::error_code ec;

        std::string_view contentType = req.getHeaderValue("content-type");

        const std::string boundaryFormat = "multipart/form-data; boundary=";
        if (!boost::starts_with(contentType, boundaryFormat))
        {
            ec.assign(static_cast<int>(ParserError::ERROR_BOUNDARY_FORMAT),
                      parserErrCategory);
            return ec;
        }

        std::string_view ctBoundary = contentType.substr(boundaryFormat.size());

        BMCWEB_LOG_DEBUG << "Boundary is \"" << ctBoundary << "\"";

        boundary = "\r\n--";
        boundary += ctBoundary;
        indexBoundary();
        lookbehind.resize(boundary.size() + 8);
        state = State::START;

        const char* buffer = req.body.data();
        size_t len = req.body.size();
        size_t prevIndex = index;
        char cl = 0;

        for (size_t i = 0; i < len; i++)
        {
            char c = buffer[i];
            switch (state)
            {
                case State::ERROR:
                    return ec;
                case State::START:
                    index = 0;
                    state = State::START_BOUNDARY;
                    [[fallthrough]];
                case State::START_BOUNDARY:
                    if (index == boundary.size() - 2)
                    {
                        if (c != cr)
                        {
                            ec.assign(static_cast<int>(
                                          ParserError::ERROR_BOUNDARY_CR),
                                      parserErrCategory);
                            return ec;
                        }
                        index++;
                        break;
                    }
                    else if (index - 1 == boundary.size() - 2)
                    {
                        if (c != lf)
                        {
                            ec.assign(static_cast<int>(
                                          ParserError::ERROR_BOUNDARY_LF),
                                      parserErrCategory);
                            return ec;
                        }
                        index = 0;

                        mime_fields.push_back({});
                        state = State::HEADER_FIELD_START;
                        break;
                    }
                    if (c != boundary[index + 2])
                    {
                        ec.assign(
                            static_cast<int>(ParserError::ERROR_BOUNDARY_DATA),
                            parserErrCategory);
                        BMCWEB_LOG_DEBUG << "Got " << c << "expecting "
                                         << boundary[index + 2];
                        return ec;
                    }
                    index++;
                    break;
                case State::HEADER_FIELD_START:
                    currentHeaderName.resize(0);
                    state = State::HEADER_FIELD;
                    headerFieldMark = i;
                    index = 0;
                    [[fallthrough]];
                case State::HEADER_FIELD:
                    if (c == cr)
                    {
                        headerFieldMark = unmarked;
                        state = State::HEADERS_ALMOST_DONE;
                        break;
                    }

                    index++;
                    if (c == hyphen)
                    {
                        break;
                    }

                    if (c == colon)
                    {
                        if (index == 1)
                        {
                            // empty header field
                            ec.assign(static_cast<int>(
                                          ParserError::ERROR_EMPTY_HEADER),
                                      parserErrCategory);
                            return ec;
                        }
                        currentHeaderName.append(buffer + headerFieldMark,
                                                 i - headerFieldMark);
                        state = State::HEADER_VALUE_START;
                        break;
                    }
                    cl = lower(c);
                    if (cl < 'a' || cl > 'z')
                    {
                        ec.assign(
                            static_cast<int>(ParserError::ERROR_HEADER_NAME),
                            parserErrCategory);
                        return ec;
                    }
                    break;
                case State::HEADER_VALUE_START:
                    if (c == space)
                    {
                        break;
                    }
                    headerValueMark = i;
                    state = State::HEADER_VALUE;
                    [[fallthrough]];
                case State::HEADER_VALUE:
                    if (c == cr)
                    {
                        std::string_view value(buffer + headerValueMark,
                                               i - headerValueMark);
                        BMCWEB_LOG_DEBUG << "Found header " << currentHeaderName
                                         << "=" << value;
                        mime_fields.rbegin()->fields.set(currentHeaderName,
                                                         value);
                        state = State::HEADER_VALUE_ALMOST_DONE;
                    }
                    break;
                case State::HEADER_VALUE_ALMOST_DONE:
                    if (c != lf)
                    {
                        ec.assign(
                            static_cast<int>(ParserError::ERROR_HEADER_VALUE),
                            parserErrCategory);
                        return ec;
                    }
                    state = State::HEADER_FIELD_START;
                    break;
                case State::HEADERS_ALMOST_DONE:
                    if (c != lf)
                    {
                        ec.assign(
                            static_cast<int>(ParserError::ERROR_HEADER_ENDING),
                            parserErrCategory);
                        return ec;
                    }
                    state = State::PART_DATA_START;
                    break;
                case State::PART_DATA_START:
                    state = State::PART_DATA;
                    partDataMark = i;
                    [[fallthrough]];
                case State::PART_DATA:
                    if (index == 0)
                    {
                        if (!skipNonBoundary(buffer, len, boundary.size() - 1,
                                             i))
                        {
                            break;
                        }
                        c = buffer[i];
                    }
                    processPartData(prevIndex, index, buffer, i, c, state,
                                    flags);
                    break;
                default:
                    return ec;
            }
        }

        if (len != 0)
        {
            mime_fields.rbegin()->content +=
                std::string_view(buffer + partDataMark, len);
        }

        return ec;
    }

    std::vector<FormPart> mime_fields;
    std::string boundary;

  private:
    void indexBoundary()
    {
        std::fill(boundaryIndex.begin(), boundaryIndex.end(), 0);
        for (const char current : boundary)
        {
            boundaryIndex[static_cast<size_t>(
                static_cast<unsigned char>(current))] = true;
        }
    }

    char lower(char c) const
    {
        return static_cast<char>(c | 0x20);
    }

    inline bool isBoundaryChar(char c) const
    {
        return boundaryIndex[static_cast<size_t>(
            static_cast<unsigned char>(c))];
    }

    bool skipNonBoundary(const char* buffer, size_t len, size_t boundaryEnd,
                         size_t& i)
    {
        // boyer-moore derived algorithm to safely skip non-boundary data
        while (i + boundary.size() <= len)
        {
            if (isBoundaryChar(buffer[i + boundaryEnd]))
            {
                break;
            }

            i += boundary.size();
        }

        return i == len ? false : true;
    }

    void processPartData(size_t& prevIndex, size_t& index, const char* buffer,
                         size_t& i, char c, State& state, int& flags)
    {
        prevIndex = index;

        if (index < boundary.size())
        {
            if (boundary[index] == c)
            {
                if (index == 0)
                {
                    if (partDataMark != unmarked)
                    {
                        mime_fields.rbegin()->content += std::string_view(
                            buffer + partDataMark, i - partDataMark);
                    }
                }
                index++;
            }
            else
            {
                index = 0;
            }
        }
        else if (index == boundary.size())
        {
            index++;
            if (c == cr)
            {
                // cr = part boundary
                flags |= partBoundary;
            }
            else if (c == hyphen)
            {
                // hyphen = end boundary
                flags |= lastBoundary;
            }
            else
            {
                index = 0;
            }
        }
        else if (index - 1 == boundary.size())
        {
            if (flags & partBoundary)
            {
                index = 0;
                if (c == lf)
                {
                    // unset the PART_BOUNDARY flag
                    flags &= ~partBoundary;
                    mime_fields.push_back({});
                    state = State::HEADER_FIELD_START;
                    return;
                }
            }
            else if (flags & lastBoundary)
            {
                if (c == hyphen)
                {
                    state = State::END;
                }
                else
                {
                    index = 0;
                }
            }
            else
            {
                index = 0;
            }
        }
        else if (index - 2 == boundary.size())
        {
            if (c == cr)
            {
                index++;
            }
            else
            {
                index = 0;
            }
        }
        else if (index - boundary.size() == 3)
        {
            index = 0;
            if (c == lf)
            {
                state = State::END;
                return;
            }
        }

        if (index > 0)
        {
            lookbehind[index - 1] = c;
        }
        else if (prevIndex > 0)
        {
            // if our boundary turned out to be rubbish, the captured lookbehind
            // belongs to partData

            mime_fields.rbegin()->content += lookbehind.substr(0, prevIndex);
            prevIndex = 0;
            partDataMark = i;

            // reconsider the current character even so it interrupted the
            // sequence it could be the beginning of a new sequence
            i--;
        }
    }

    std::string currentHeaderName;
    std::string currentHeaderValue;

    static constexpr char cr = '\r';
    static constexpr char lf = '\n';
    static constexpr char space = ' ';
    static constexpr char hyphen = '-';
    static constexpr char colon = ':';
    static constexpr size_t unmarked = -1U;

    static constexpr int partBoundary = 1;
    static constexpr int lastBoundary = 2;

    std::array<bool, 256> boundaryIndex;
    std::string lookbehind;
    State state;
    int flags = 0;
    size_t index = 0;
    size_t headerFieldMark = unmarked;
    size_t headerValueMark = unmarked;
    size_t partDataMark = unmarked;
};

#include <xcontainer.hpp>

inline namespace XContainer
{
    XString trim(XString const & str) noexcept {
        auto const start{str.find_first_not_of(" \t\r\n")};
        if (XString::npos == start) {
            return {};
        }
        auto const end{ str.find_last_not_of(" \t\r\n") };
        return str.substr(start,end - start + 1);
    }

    XStringVector split(XString const & str, char const delimiter) noexcept {
        XIStringStream ss {str};
        XStringVector tokens {};
        for (XString token {};std::getline(ss, token, delimiter);)
        { tokens.push_back(std::move(token)); }
        return tokens;
    }
}

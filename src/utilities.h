#ifndef UTILITIES_H
#define UTILITIES_H

#include <Windows.h>
#include <string>

#include "imgui/imgui.h"

namespace string_utils {

    wchar_t* cstr_to_wcstr(const char* c);
    std::string wcstr_to_string(const wchar_t* wc);
    const wchar_t* string_to_wcstr(std::string s);
    std::string wstring_to_string(const std::wstring& ws);
    std::wstring string_to_wstring(const std::string& s);

    std::string replace_substr(std::string str, std::string subStrCurr, std::string subStrNew, size_t pos = 0);

} // namespace string_utils

namespace url_utils {

    std::string getScheme(const std::string& url);
    std::string getDomain(const std::string& url);
    std::string getPath(const std::string& url);

} // namespace url_utils

namespace utils {

namespace font {

    ImFont* GetFont(const std::string& fontFilePath, float fontSize);
    void ReleaseFonts();

} // namespace font

} // namespace utils

#endif /* UTILITIES_H */

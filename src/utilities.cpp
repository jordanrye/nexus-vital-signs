#include "utilities.h"

#include <map>

#include "shared.h"

namespace string_utils 
{
    wchar_t* cstr_to_wcstr(const char* c)
    {
        const size_t cSize = strlen(c);
        const size_t wcSize = cSize + 1;
        wchar_t* wc = new wchar_t[wcSize];
        mbstowcs_s(NULL, wc, wcSize, c, cSize);
        return wc;
    }

    std::string wcstr_to_string(const wchar_t* wc)
    {
        std::wstring ws(wc);
        std::string s(ws.begin(), ws.end());
        return s;
    }

    const wchar_t* string_to_wcstr(std::string s)
    {
        std::wstring ws = std::wstring(s.begin(), s.end());
        const wchar_t* wc = ws.c_str();
        return wc;
    }
    
    std::string wstring_to_string(const std::wstring& ws)
    {
        if (ws.empty())
        {
            return std::string();
        }

        int sz = WideCharToMultiByte(CP_UTF8, 0, &ws[0], (int)ws.length(), 0, 0, 0, 0);
        std::string s(sz, 0);
        WideCharToMultiByte(CP_UTF8, 0, &ws[0], (int)ws.length(), &s[0], sz, 0, 0);
        return s;
    }

    std::wstring string_to_wstring(const std::string& s)
    {
        if (s.empty())
        {
            return std::wstring();
        }

        int sz = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.length(), 0, 0);
        std::wstring ws(sz, 0);
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.length(), &ws[0], (int)ws.length());
        return ws;
    }

    std::string replace_substr(std::string str, std::string subStrCurr, std::string subStrNew, size_t pos)
    {
        while ((pos = str.find(subStrCurr, pos)) != std::string::npos) {
            str.replace(pos, subStrCurr.size(), subStrNew);
            pos += subStrNew.size();
        }
        return str;
    }

} // namespace string_utils

namespace url_utils {

    std::string getScheme(const std::string& url)
    {
        std::string scheme = "";

        if (url.find("http://") == 0)
        {
            scheme = "http://";
        }
        else if (url.find("https://") == 0)
        {
            scheme = "https://";
        }

        return scheme;
    }

    std::string getDomain(const std::string& url)
    {
        size_t idxStart = getScheme(url).length();
        size_t idxEnd = 0;

        if (idxStart > 0)
        {
            idxEnd = url.find("/", idxStart);
        }

        return url.substr(idxStart, idxEnd);
    }

    std::string getPath(const std::string& url)
    {
        size_t lenScheme = getScheme(url).length();
        size_t lenDomain = getDomain(url).length();
        return url.substr(lenScheme + lenDomain);
    }

} // namespace url_utils

namespace utils {

namespace font {

    static std::map<std::string, ImFont*> m_FontsCache;

    static void ReceiveFont(const char* aIdentifier, void* aFont)
    {
        std::string key(aIdentifier);
        m_FontsCache[key] = (ImFont*)aFont;
    }

    ImFont* GetFont(const std::string& fontPath, float fontSize)
    {
        if (fontPath.empty()) return nullptr;

        std::string key = fontPath + "_" + std::to_string(fontSize) + "px";
        if (m_FontsCache.find(key) != m_FontsCache.end())
        {
            return m_FontsCache[key];
        }

        ImFont* font = nullptr;
        std::filesystem::path path(fontPath);
        if (path.is_relative())
        {
            path = GameDir / path;
        }

        if (std::filesystem::exists(path))
        {
            m_FontsCache[key] = nullptr;
            APIDefs->Fonts.AddFromFile(key.c_str(), fontSize, path.string().c_str(), ReceiveFont, nullptr);
        }

        return font;
    }

    void ReleaseFonts()
    {
        if (nullptr != APIDefs)
        {
            for (auto const& [key, val] : m_FontsCache)
            {
                APIDefs->Fonts.Release(key.c_str(), ReceiveFont);
            }
            m_FontsCache.clear();
        }
    }

} // namespace font

} // namespace utils
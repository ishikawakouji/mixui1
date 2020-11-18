#include <Windows.h>
#include <vector>
#include <iostream>
#include "utf8sjis.h"

/*
* utf8 -> wide
*/
std::wstring utf8_to_wide_winapi(std::string const& src)
{
    auto const dest_size = ::MultiByteToWideChar(CP_UTF8, 0U, src.data(), -1, nullptr, 0U);
    std::vector<wchar_t> dest(dest_size, L'\0');
    if (::MultiByteToWideChar(CP_UTF8, 0U, src.data(), -1, dest.data(), (int)dest.size()) == 0) {
        throw std::system_error{ static_cast<int>(::GetLastError()), std::system_category() };
    }
    dest.resize(std::char_traits<wchar_t>::length(dest.data()));
    dest.shrink_to_fit();
    return std::wstring(dest.begin(), dest.end());
}

/*
* wide -> SJIS
*/
std::string wide_to_multi_winapi(std::wstring const& src)
{
    auto const dest_size = ::WideCharToMultiByte(CP_ACP, 0U, src.data(), -1, nullptr, 0, nullptr, nullptr);
    std::vector<char> dest(dest_size, '\0');
    if (::WideCharToMultiByte(CP_ACP, 0U, src.data(), -1, dest.data(), (int)dest.size(), nullptr, nullptr) == 0) {
        throw std::system_error{ static_cast<int>(::GetLastError()), std::system_category() };
    }
    dest.resize(std::char_traits<char>::length(dest.data()));
    dest.shrink_to_fit();
    return std::string(dest.begin(), dest.end());
}

/*
* utf8 -> wide -> SJIS
*/
std::string utf8_to_wide_to_multi_winapi(std::string const& src)
{
    std::wstring wval = utf8_to_wide_winapi(src);
    return wide_to_multi_winapi(wval);
}
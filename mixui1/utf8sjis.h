#pragma once
/*
* utf8, wide, Shift-JIS ‚ð‘ŠŒÝ‚É•ÏŠ·‚·‚é•û–@‚Ìˆê•”‚ðŽÀ‘•
* ref https://nekko1119.hatenablog.com/entry/2017/01/02/054629
*/

#include <string>

/*
* utf8 -> wide -> SJIS
*/
extern std::string utf8_to_wide_to_multi_winapi(std::string const& src);

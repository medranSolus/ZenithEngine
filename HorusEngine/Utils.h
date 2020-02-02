#pragma once
#include <string>
#include <boost/locale/encoding.hpp>

inline std::wstring toUtf8(const std::string & s)
{
	return boost::locale::conv::to_utf<wchar_t>(s, "UTF-8");
}
#pragma once

#include <cstdlib>
#include <string>

#define $ (*this)

// Depends on Locale
namespace liquid
{
  namespace
  {
    // From wide to multibyte
    inline void wide2multi(const std::wstring & src, std::string & dest)
    {
      char * mbs = new char[src.length() * MB_CUR_MAX + 1];
      std::wcstombs(mbs, src.c_str(), src.length() * MB_CUR_MAX + 1);
      dest = mbs;
      delete [] mbs;
    }

    // From multibyte to wide
    inline void multi2wide(const std::string & src, std::wstring & dest)
    {
      wchar_t * wcs = new wchar_t[src.length() + 1];
      std::mbstowcs(wcs, src.c_str(), src.length() + 1);
      dest = wcs;
      delete [] wcs;
    }
  }
}

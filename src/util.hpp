#pragma once

#include <cstdlib>
#include <string>

// Depends on Locale
namespace liquid
{
  // From wide to multibyte
  void wide2multi(const std::wstring & src, std::string & dest) 
  {
    char * mbs = new char[src.length() * MB_CUR_MAX + 1];
    std::wcstombs(mbs, src.c_str(), src.length() * MB_CUR_MAX + 1);
    dest = mbs;
    delete [] mbs;
  }

  // From multibyte to wide
  void multi2wide(const std::string & src, std::wstring & dest)
  {
    wchar_t * wcs = new wchar_t[src.length() + 1];
    std::mbstowcs(wcs, src.c_str(), src.length() + 1);
    dest = wcs;
    delete [] wcs;
  }
}


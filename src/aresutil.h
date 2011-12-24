#pragma once

#include <string>
#include <iterator>
#include "ares.h"

namespace ares
{
  namespace
  {
    size_t u8strlen(const std::string & str)
    {
      size_t size = 0;
      for(auto itr=str.begin(); itr != str.end();)
      {
        const unsigned char c = *itr;
        if(false) ;
        else if(c < 0x80) std::advance(itr, 1);
        else if(c < 0xe0) std::advance(itr, 2);
        else if(c < 0xf0) std::advance(itr, 3);
        else if(c < 0xf8) std::advance(itr, 4);
        else if(c < 0xfc) std::advance(itr, 5);
        else if(c < 0xfe) std::advance(itr, 6);
        else { ++itr; continue; }
        ++size;
      }
      return size;
    }
  }
}

#pragma once

#include "util.hpp"
#include "ares.h"

namespace ares
{
  struct CSegment
  {
    station_id_t begin, end;
    line_id_t line;

    CSegment(station_id_t begin)
      : begin(begin),
        end(0),
        line(0) {}

    CSegment(station_id_t begin,
             line_id_t line,
             station_id_t end)
      : begin(begin),
        end(end),
        line(line) {}

    /**
     * Check whether this segment is begin.
     */
    bool is_begin() const {
      return line == 0 && end == 0;
    }

    bool operator==(const CSegment & b) const {
      const CSegment & a = *this;
      return a.begin == b.begin && a.end == b.end && a.line == b.line;
    }

    bool reverse() {
      if($.is_begin()) return false;
      std::swap($.begin, $.end);
      return true;
    }
  };
}

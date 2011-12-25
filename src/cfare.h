#pragma once

#include "ckilo.h"

namespace ares
{
  enum FARE_MODE
  {
    FARE_ADULT     = 0,
    FARE_CHILD     = 1,
    FARE_STUDENT   = 2,
    FARE_ROUNDTRIP = 4,
  };

  /**
   * 運賃を表現する構造体.
   * JRの運賃は学割や周遊アプローチ券割引が適用されるが,
   * 社線には適用されないといったことを抽象化したい.
   */
  struct CFare
  {
    const CKilo & kilo;
    int JR, other;

    explicit CFare(const CKilo & kilo) : kilo(kilo),
                                         JR(kilo.get_additional_fare_JR()),
                                         other(kilo.get_fare_private()) {}

    int get_fare(FARE_MODE = FARE_ADULT) const
    {
      return JR + other;
    }

    operator int()
    {
      return $.get_fare();
    }
  };
}

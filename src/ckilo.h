#pragma once

#include "util.hpp"
#include <stdexcept>

namespace ares
{
  enum COMPANY_TYPE {
    COMPANY_HONSHU,
    COMPANY_HOKKAIDO,
    COMPANY_KYUSHU,
    COMPANY_SHIKOKU,
    MAX_COMPANY_TYPE,
  };
  enum LINE_TYPE {
    LINE_LOCAL,
    LINE_MAIN,
    MAX_LINE_TYPE,
  };

  namespace
  {
    const char * COMPANY_TYPE_LABEL[ares::MAX_COMPANY_TYPE] =
    {
      "本州　",
      "北海道",
      "九州　",
      "四国　",
    };

    const char * LINE_TYPE_LABEL[MAX_COMPANY_TYPE] =
    {
      "地方交通線",
      "幹線",
    };
  }

  class CKilo
  {
  public:

    friend std::ostream & operator<<(std::ostream & ost, CKilo & kilo) {
      for(int i=0; i != MAX_COMPANY_TYPE; ++i)
      {
        ost << COMPANY_TYPE_LABEL[i] << ": ";
        for(int j=MAX_LINE_TYPE; j; )
        {
          --j;
          ost << LINE_TYPE_LABEL[j] << ": "
              << kilo.kilo[i][j] << " ";
        }
        ost << '\n';
      }
      return ost;
    }

  private:

    int kilo[MAX_COMPANY_TYPE][MAX_LINE_TYPE];

    void check_boundary(size_t i) const {
      if(i >= MAX_COMPANY_TYPE)
      {
        throw std::out_of_range("too large company index");
      }
    }

  public:

    CKilo() : kilo({{0}}) {}

    void add(size_t i, bool is_main, int kilo) {
      $.set(i, is_main, $.get(i, is_main) + kilo);
    }

    int get(size_t i, bool is_main) const {
      check_boundary(i);
      return $.kilo[i][is_main ? LINE_MAIN : LINE_LOCAL];
    }

    int get_kilo(size_t i, bool is_main) const {
      const int kilo10 = $.get(i, is_main);
      return kilo10 / 10 + (kilo10 % 10 ? 1 : 0);
    }

    void set(size_t i, bool is_main, int kilo) {
      check_boundary(i);
      $.kilo[i][is_main ? LINE_MAIN : LINE_LOCAL] = kilo;
    }

    bool is_zero(size_t i) {
      check_boundary(i);
      return (kilo[i][0] == 0 && kilo[i][1] == 0);
    }

    // bool is_zero_honshu()   { return is_zero(0); }
    // bool is_zero_hokkaido() { return is_zero(1); }
    // bool is_zero_kyushu()   { return is_zero(2); }
    // bool is_zero_shikoku()  { return is_zero(3); }

    bool is_only(size_t idx) {
      check_boundary(idx);
      for(size_t i=0; i<MAX_COMPANY_TYPE; ++i)
      {
        if(i != idx && !is_zero(i)) { return false; }
      }
      return true;
    }
  };
}

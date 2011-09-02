#include <cmath>
#include "croute.h"

namespace ares
{
  namespace
  {
    const double FARE_TAX = 1.05;
    const int FARE_KILO_HONSHU_MAIN[] = {0, 300, 600};
    const double FARE_RATE_HONSHU_MAIN[] = {16.2, 12.85, 7.05};

    inline int round(int val)
    {
      int base = static_cast<int>(val * FARE_TAX) + 5;
      return base - base % 10;
    }
  }

  void CRoute::append_route(line_id_t line, station_id_t station)
  {
    way.push_back(std::pair<line_id_t, station_id_t>(line, station));
  }

  bool CRoute::is_valid() const
  {
    // check way[ i ].second in way[i].first AND
    // check way[i-1].second in way[i].first
    for(size_t i = 1; i < way.size(); ++i)
    {
      way[i].second;
    }
  }

  int CRoute::calc_fare_inplace()
  {
    return 0;
  }

  int CRoute::calc_honshu_main(int kilo)
  {
    // hard-coded.
    // I should use SQLite database.
    if (kilo <= 3)
      return 140;
    else if (kilo <=6)
      return 180;
    else if (kilo <=10)
      return 190;

    // align kilo.
    if (kilo <= 50)
      kilo = (kilo-1)/5*5+3;
    else if (kilo <= 100)
      kilo = (kilo-1)/10*10+5;
    else if (kilo <= 600)
      kilo = (kilo-1)/20*20+10;
    else
      kilo = (kilo-1)/40*40+20;

    // sum fare.
    double sum[] = {0, 0, 0};
    sum[0] = (kilo < FARE_KILO_HONSHU_MAIN[1])
      ? FARE_RATE_HONSHU_MAIN[0] * kilo
      : FARE_RATE_HONSHU_MAIN[0] * FARE_KILO_HONSHU_MAIN[1];
    if (kilo <= FARE_KILO_HONSHU_MAIN[1])
      sum[1] =  0;
    else if (kilo < FARE_KILO_HONSHU_MAIN[2])
      sum[1] = FARE_RATE_HONSHU_MAIN[1] * (kilo - FARE_KILO_HONSHU_MAIN[1]);
    else
      sum[1] = FARE_RATE_HONSHU_MAIN[1] * (FARE_KILO_HONSHU_MAIN[2] - FARE_KILO_HONSHU_MAIN[1]);
    sum[2] = (kilo <= FARE_KILO_HONSHU_MAIN[2]) ? 0 :
      FARE_RATE_HONSHU_MAIN[2] * (kilo - FARE_KILO_HONSHU_MAIN[2]);

    const double totalsum = sum[0] + sum[1] + sum[2];
    if(kilo < 100)
    {
      return round(static_cast<int>(std::ceil(totalsum/10)*10));
    }
    else
    {
      const int isum = static_cast<int>(totalsum) + 50;
      return round(isum - isum % 100);
    }
  }
}

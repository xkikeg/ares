#include <cmath>
#include "util.hpp"
#include "croute.h"
#include "cdatabase.h"

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

    template<class MainLineLookupFunction,
             class LocalLineLookupFunction>
    int calc_fare_as_honshu(const CDatabase & db,
                            const CKilo & kilo,
                            const boost::optional<COMPANY_TYPE> comp_type,
                            MainLineLookupFunction func_main,
                            LocalLineLookupFunction func_local)
    {
      const CHecto hecto_main  =
        comp_type ? kilo.get(*comp_type, true ) : kilo.get_all(true);
      const CHecto hecto_local =
        comp_type ? kilo.get(*comp_type, false) : kilo.get_all(false);
      const CHecto hecto_local_fake = comp_type
        ? kilo.get(*comp_type, false, false) : kilo.get_all(false, false);
      // Main only
      if(hecto_local == 0)
      {
        const DENSHA_SPECIAL_TYPE denshaid = kilo.get_densha_and_circleid();
        const char * faretable=nullptr;
        switch(denshaid)
        {
        case DENSHA_SPECIAL_TOKYO:      faretable = "D1"; break;
        case DENSHA_SPECIAL_OSAKA:      faretable = "D2"; break;
        case DENSHA_SPECIAL_YAMANOTE:   faretable = "E1"; break;
        case DENSHA_SPECIAL_OSAKAKANJO: faretable = "E2"; break;
        default:
          return func_main(hecto_main);
        }
        return db.get_fare_table(faretable,
                                 COMPANY_HONSHU,
                                 hecto_main);
      }
      // Local only or (Main+Local)<=10
      if(hecto_main == 0 || (hecto_main + hecto_local) <= 10)
      {
        return func_local(hecto_main + hecto_local);
      }
      // Both main & local
      const int hecto_total =
        hecto_main + hecto_local_fake;
      return func_main(hecto_total);
    }
  }

  std::ostream & operator<<(std::ostream & ost, const ares::CRoute & route)
  {
    if(route.way.empty()) return ost;
    for(size_t i=0; i<route.way.size(); ++i)
    {
      if(i==0)
      {
        ost << route.db->get_station_name(route.way[i].begin);
      }
      else
      {
        ost << "[" << route.db->get_station_name(route.way[i].begin) << "]";
      }
      ost << ",";
      ost << route.db->get_line_name(route.way[i].line) << ",";
    }
    ost << route.db->get_station_name(route.way.back().end);
    return ost;
  }

  bool CRoute::operator==(const CRoute & b) const {
    const CRoute & a = *this;
    return a.way == b.way;
  }

  void CRoute::append_route(line_id_t line, station_id_t station)
  {
    if(way.back().is_begin())
    {
      way.back().line = line;
      way.back().end = station;
    }
    else {
      way.push_back(CSegment(way.back().end, line, station));
    }
  }

  void CRoute::append_route(const char * line, const char * station)
  {
    $.append_route(db->get_lineid(line),
                   db->get_stationid(station));
  }

  void CRoute::append_route(line_id_t line, station_id_t begin, station_id_t end)
  {
    way.push_back(CSegment(begin, line, end));
  }

  void CRoute::append_route(const char * line, const char * begin, const char * end)
  {
    $.append_route(db->get_lineid(line),
                   db->get_stationid(begin),
                   db->get_stationid(end));
  }

  bool CRoute::is_contains(station_id_t station) const
  {
    for(auto itr = $.begin(); itr != $.end(); ++itr)
    {
      if($.db->is_contains(*itr, station)) return true;
    }
    return false;
  }

  bool CRoute::is_valid() const
  {
    // Connectivity: each segment tail = next segment head
    for(auto itr=$.begin()+1; itr != $.end(); ++itr)
    {
      if((itr-1)->end != itr->begin){ return false; }
    }
    // NoOverlap   : segments does not overlap
    std::map<line_id_t, liquid::UniqueIntervalTree<station_id_t> > checktree;
    for(auto itr=$.begin(); itr != $.end(); ++itr)
    {
      if(!$.db->is_belong_to_line(itr->line, itr->begin) ||
         !$.db->is_belong_to_line(itr->line, itr->end  ) ||
         !checktree[itr->line].insert(
           $.db->get_range(itr->line, itr->begin, itr->end))) { return false; }
    }
    return true;
  }

  class CKilo CRoute::get_kilo() const
  {
    CKilo kilo;
    std::vector<CKiloValue> result;
    for(auto itr=$.begin(); itr != $.end(); ++itr)
    {
      bool is_main;
      DENSHA_SPECIAL_TYPE denshaid, circleid;
      result.clear();
      bool ret = $.db->get_company_and_kilo(itr->line,
                                            itr->begin,
                                            itr->end,
                                            result,
                                            is_main,
                                            denshaid,
                                            circleid);
      if(!ret) { return CKilo(); }
      kilo.update_denshaid(denshaid, circleid);
      for(auto j=result.begin(); j != result.end(); ++j)
      {
        kilo.add(j->company, is_main, j->begin, j->end);
      }
    }
    return std::move(kilo);
  }

  /*
   * @note この実装は三島会社同士で直接連絡できないことを前提にしている.
   * 例えば大分から愛媛のJR路線が出来れば面倒, なんてね.
   */
  int CRoute::calc_fare_inplace()
  {
    using namespace std::placeholders;
    // Error checking, returning -1 is not good, boost::optional is better.
    if(!$.is_valid()) { return -1; }
    // Rewrite Route: shinkansen / route-variant
    // Get Kilo: Additional fare should included in CKilo
    const CKilo kilo($.get_kilo());
    // 0キロ
    if(kilo.is_all_zero()) { return 0; }
    // Get only company.
    boost::optional<COMPANY_TYPE> only = kilo.get_only();
    // JR四国 or JR九州
    if(only && (*only == COMPANY_KYUSHU || *only == COMPANY_SHIKOKU))
    {
      const CHecto hecto_main  = kilo.get(*only, true );
      const CHecto hecto_local = kilo.get(*only, false);
      const CHecto hecto_lfake = kilo.get(*only, false, false);
      // only 幹線
      if(hecto_local == 0)
      {
        return $.db->get_fare_table("C1", *only, hecto_main);
      }
      // only 地方交通線
      else if(hecto_main == 0)
      {
        boost::optional<int> special_fare =
          $.db->get_fare_country_table("C2", *only, hecto_local, hecto_lfake);
        if(special_fare) { return *special_fare; }
        return $.db->get_fare_table("C1", *only, hecto_lfake);
      }
      else
      {
        boost::optional<int> special_fare =
          $.db->get_fare_country_table("C3", *only,
                                       hecto_main + hecto_local,
                                       hecto_main + hecto_lfake);
        if(special_fare) { return *special_fare; }
        return $.db->get_fare_table("C1", *only, hecto_main + hecto_lfake);
      }
    }
    // JR北海道
    else if(only && (*only == COMPANY_HOKKAIDO))
    {
      return calc_fare_as_honshu(*db, kilo,
                                 COMPANY_HOKKAIDO,
                                 std::bind(&CDatabase::get_fare_table,
                                           $.db, "C1", COMPANY_HOKKAIDO, _1),
                                 std::bind(&CDatabase::get_fare_table,
                                           $.db, "B1", COMPANY_HOKKAIDO, _1));
    }
    // 本州含み
    else
    {
      const int base_fare =
        calc_fare_as_honshu(*db, kilo,
                            boost::none,
                            std::bind(CRoute::calc_honshu_main, _1),
                            std::bind(&CDatabase::get_fare_table,
                                      $.db, "B1", COMPANY_HONSHU, _1));
      int add_fare = 0;
      for(size_t i=COMPANY_HOKKAIDO; i < MAX_COMPANY_TYPE; ++i)
      {
        if(!kilo.is_zero(i))
          add_fare +=
            calc_fare_as_honshu(*db, kilo,
                                COMPANY_TYPE(i),
                                std::bind(&CDatabase::get_fare_table,
                                          $.db, "A2", i, _1),
                                std::bind(&CDatabase::get_fare_table,
                                          $.db, "B2", i, _1));
      }
      return base_fare + add_fare;
    }
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

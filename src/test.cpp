#include <iostream>
#include <locale>
#include <memory>
#include <stdexcept>
#include <cstdlib>
#include <boost/foreach.hpp>
#include "cdatabase.h"
#include "croute.h"
#include "sqlite3_wrapper.h"

#define teststation(str, col, mode)                                 \
  std::cout << #col << ':' << #mode << ':' << str << '\n';          \
  db->find_stationid_with_ ## col(str, ares::FIND_##mode, sresult);	\
  BOOST_FOREACH(ares::station_id_t & e, sresult)                    \
  {                                                                 \
    std::cout << e << '\t'                                          \
              << db->get_station_name(e) << '\t'                    \
              << db->get_station_yomi(e) << '\t'                    \
              << db->get_station_denryaku(e) << '\n';               \
  }                                                                 \
  sresult.clear();

void testdb(std::shared_ptr<ares::CDatabase> db)
{
  ares::station_vector sresult;
  teststation("鹿児島", name, PREFIX);
  teststation("はっさむ", yomi, PREFIX);
  teststation("ミフ", denryaku, EXACT);
  teststation("セカオ", denryaku, PREFIX);

  ares::connect_vector rresult;
  std::cout << "connect:鹿児島1\n";
  ares::line_id_t l = db->get_lineid("鹿児島1");
  db->find_connect_line(l, rresult);
  BOOST_FOREACH(ares::station_fqdn_t & r, rresult)
  {
    std::cout << r.first << '\t'
              << db->get_line_name(r.first) << '\t'
              << r.second << '\t'
              << db->get_station_name(r.second) << '\n';
  }
}

void testroute(std::shared_ptr<ares::CDatabase> db)
{
  ares::CRoute route(db, db->get_stationid("東京"));
  route.append_route(db->get_lineid   ("東海道"),
                     db->get_stationid("神戸"));
  route.append_route(db->get_lineid   ("山陽"),
                     db->get_stationid("岡山"));
  route.calc_fare_inplace();
}

// Usage: ./test db
int main(int argc, char ** argv)
{
  if(argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " dbfile" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  std::locale loc("");
  std::locale::global(loc);
  std::cin.imbue(loc);
  std::wcin.imbue(loc);
  std::cout.imbue(loc);
  std::wcout.imbue(loc);
  std::cerr.imbue(loc);
  std::wcerr.imbue(loc);
  std::shared_ptr<ares::CDatabase> db;
  try
  {
    db.reset(new ares::CDatabase(argv[1]));
  }
  catch(ares::IOException & e)
  {
    std::exit(EXIT_FAILURE);
  }
  testdb(db);
  testroute(db);
}

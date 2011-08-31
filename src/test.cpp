#include <iostream>
#include <locale>
#include <memory>
#include <stdexcept>
#include <cstdlib>
#include <boost/foreach.hpp>
#include "cdatabase.h"
#include "croute.h"

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

  ares::line_vector lresult;
  ares::route_vector rresult;
  std::cout << "connect:東海道\n";
  db->find_lineid_with_name("東海道", ares::FIND_EXACT, lresult);
  try
  {
    ares::line_id_t l = lresult.at(0);
    lresult.clear();
    db->find_connect_line(l, rresult);
    BOOST_FOREACH(ares::route_pair & r, rresult)
	{
	  std::cout << r.first << '\t'
                << db->get_line_name(r.first) << '\t'
                << r.second << '\t'
                << db->get_station_name(r.second) << '\n';
	}
  }
  catch(const std::out_of_range & e)
  {
    std::cerr << "Line not found.\n";
  }
}

void testroute(std::shared_ptr<ares::CDatabase> db)
{
  ares::line_vector lines;
  ares::station_vector stations;
  db->find_stationid_with_name("東京", ares::FIND_EXACT, stations);
  ares::CRoute route(stations.at(0), db);
  stations.clear();
  db->find_lineid_with_name("東海道", ares::FIND_EXACT, lines);
  db->find_stationid_with_name("神戸", ares::FIND_EXACT, stations);
  route.append_route(lines.at(0), stations.at(0));
  lines.clear(); stations.clear();
  db->find_lineid_with_name("山陽", ares::FIND_EXACT, lines);
  db->find_stationid_with_name("岡山", ares::FIND_EXACT, stations);
  route.append_route(lines.at(0), stations.at(0));
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

#include <iostream>
#include <locale>
#include <memory>
#include <stdexcept>
#include <cstdlib>
#include <boost/foreach.hpp>
#include "cdatabase.h"

void testdb(const char * filename)
{
  std::unique_ptr<ares::CDatabase> db;
  try
    {
      db.reset(new ares::CDatabase(filename));
    }
  catch(ares::IOException & e)
    {
      exit(EXIT_FAILURE);
    }
  ares::station_vector sresult;
  std::cout << "name:PREFIX:鹿児島" << std::endl;
  db->search_station_with_name("鹿児島", ares::SEARCH_PREFIX, sresult);
  BOOST_FOREACH(ares::station_id_t & e, sresult)
    {
      std::cout << e << '\t' << db->get_station_name(e) << std::endl;
    }
  sresult.clear();
  std::cout << "yomi:PREFIX:はっさむ" << std::endl;
  db->search_station_with_yomi("はっさむ", ares::SEARCH_PREFIX, sresult);
  BOOST_FOREACH(ares::station_id_t & e, sresult)
    {
      std::cout << e << '\t' << db->get_station_name(e) << std::endl;
    }
  sresult.clear();
  std::cout << "denryaku:EXACT:ミフ" << std::endl;
  db->search_station_with_denryaku("ミフ", ares::SEARCH_EXACT, sresult);
  BOOST_FOREACH(ares::station_id_t & e, sresult)
    {
      std::cout << e << '\t' << db->get_station_name(e) << std::endl;
    }
  sresult.clear();
  std::cout << "denryaku:PREFIX:セカオ" << std::endl;
  db->search_station_with_denryaku("セカオ", ares::SEARCH_PREFIX, sresult);
  BOOST_FOREACH(ares::station_id_t & e, sresult)
    {
      std::cout << e << '\t' << db->get_station_name(e) << std::endl;
    }
  ares::station_vector lresult;
  std::cout << "connect:東海道\n";
  db->search_line_with_name("東海道", ares::SEARCH_EXACT, lresult);
  try
    {
      ares::line_id_t l = lresult.at(0);
      lresult.clear();
      db->search_connect_line(l, lresult);
      BOOST_FOREACH(ares::line_id_t & e, lresult)
	{
	  std::cout << e << '\t' << db->get_line_name(e) << std::endl;
	}
    }
  catch(const std::out_of_range & e)
    {
      std::cerr << "Line not found.\n";
    }
}

// Usage: ./test db
int main(int argc, char ** argv)
{
  if(argc < 2)
    {
      std::cerr << "Usage: " << argv[0] << " dbfile" << std::endl;
      std::exit(EXIT_FAILURE);
    }

  const char * locale = std::setlocale(LC_ALL, "");
  std::cerr << "Locale: " << locale << std::endl;
  testdb(argv[1]);
}

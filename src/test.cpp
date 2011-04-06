#include <iostream>
#include <locale>
#include <cstdlib>
#include <boost/foreach.hpp>
#include "cdatabase.h"

void testdb(const char * filename)
{
  try
    {
      ares::CDatabase db(filename);
      ares::station_vector result;
      std::cout << "name:PREFIX:鹿児島" << std::endl;
      db.search_station_with_name("鹿児島", ares::SEARCH_PREFIX, result);
      BOOST_FOREACH(ares::station_id_t & e, result)
	{
	  std::cout << e << '\t' << db.get_station_name(e) << std::endl;
	}
      result.clear();
      std::cout << "yomi:PREFIX:はっさむ" << std::endl;
      db.search_station_with_yomi("はっさむ", ares::SEARCH_PREFIX, result);
      BOOST_FOREACH(ares::station_id_t & e, result)
	{
	  std::cout << e << '\t' << db.get_station_name(e) << std::endl;
	}
      result.clear();
      std::cout << "denryaku:EXACT:ミフ" << std::endl;
      db.search_station_with_denryaku("ミフ", ares::SEARCH_EXACT, result);
      BOOST_FOREACH(ares::station_id_t & e, result)
	{
	  std::cout << e << '\t' << db.get_station_name(e) << std::endl;
	}
      result.clear();
      std::cout << "denryaku:PREFIX:セカオ" << std::endl;
      db.search_station_with_denryaku("セカオ", ares::SEARCH_PREFIX, result);
      BOOST_FOREACH(ares::station_id_t & e, result)
	{
	  std::cout << e << '\t' << db.get_station_name(e) << std::endl;
	}
    }
  catch(ares::IOException & e)
    {
      exit(EXIT_FAILURE);
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

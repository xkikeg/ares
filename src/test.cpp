#include <iostream>
#include <cstdlib>
#include <boost/foreach.hpp>
#include "cdatabase.h"

void testdb(const char * filename)
{
  try
    {
      ares::CDatabase db(filename);
      ares::line_vector result;
      std::cout << "鹿児島" << std::endl;
      db.search_station_with_name("鹿児島", ares::SEARCH_PREFIX, result);
      BOOST_FOREACH(ares::line_id_t & e, result)
	{
	  std::cout << e << std::endl;
	}
      result.clear();
      std::cout << "はっさむ" << std::endl;
      db.search_station_with_yomi("はっさむ", ares::SEARCH_PREFIX, result);
      BOOST_FOREACH(ares::line_id_t & e, result)
	{
	  std::cout << e << std::endl;
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
  testdb(argv[1]);
}

#include <iostream>
#include <locale>
#include <memory>
#include <stdexcept>
#include <cstdlib>
#include <boost/foreach.hpp>
#include "cdatabase.h"

#define teststation(str, col, mode)					\
  std::cout << #col << ':' << #mode << ':' << str << '\n';		\
  db->search_station_with_ ## col(str, ares::SEARCH_##mode, sresult);	\
  BOOST_FOREACH(ares::station_id_t & e, sresult)			\
  {									\
    std::cout << e << '\t'						\
	      << db->get_station_name(e) << '\t'			\
	      << db->get_station_yomi(e) << '\t'			\
	      << db->get_station_denryaku(e) << '\n';			\
  }									\
  sresult.clear();

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
  teststation("鹿児島", name, PREFIX);
  teststation("はっさむ", yomi, PREFIX);
  teststation("ミフ", denryaku, EXACT);
  teststation("セカオ", denryaku, PREFIX);

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

  std::locale loc("");
  std::locale::global(loc);
  std::cin.imbue(loc);
  std::wcin.imbue(loc);
  std::cout.imbue(loc);
  std::wcout.imbue(loc);
  std::cerr.imbue(loc);
  std::wcerr.imbue(loc);
  testdb(argv[1]);
}

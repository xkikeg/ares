#include <iostream>
#include <locale>
#include <memory>
#include <stdexcept>
#include <cstdlib>
#include <boost/foreach.hpp>
#include "cdatabase.h"
#include "croute.h"
#include "sqlite3_wrapper.h"

class ExitWithUsage : public std::runtime_error
{
public:
  ExitWithUsage() : std::runtime_error("Error to show usage") {}
};

void calc_route(std::shared_ptr<ares::CDatabase> db,
                int argc,
                char ** argv)
{
  if(argc == 0) { throw ExitWithUsage(); }
  ares::CRoute route(db, db->get_stationid(argv[0]));
  int i=1;
  do
  {
    if(i+1 >= argc) { throw ExitWithUsage(); }
    route.append_route(db->get_lineid   (argv[i  ]),
                       db->get_stationid(argv[i+1]));
    i += 2;
  } while(i < argc);
  int fare = route.calc_fare_inplace();
  std::cout << fare << std::endl;
}

int main(int argc, char ** argv)
{
  try
  {
    if(argc < 2) { throw ExitWithUsage(); }
    std::shared_ptr<ares::CDatabase> db;
    try { db.reset(new ares::CDatabase(argv[1])); }
    catch(ares::IOException & e)
    {
      std::cerr << "DB file " << argv[1] << " not found" << std::endl;
      throw ExitWithUsage();
    }
    calc_route(db, argc - 2, argv + 2);
  }
  catch(const ExitWithUsage & e)
  {
    std::cerr << "Usage: " << argv[0] << " dbfile"
              << " station1" << " line1" << " station2"
              << " ... stationN" << std::endl;
    std::exit(EXIT_FAILURE);
  }
  return 0;
}

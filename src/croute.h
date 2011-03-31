#include <boost/shared_ptr.hpp>
#include "cdatabase.h"

namespace ares
{
  /**
   * Class represents a route.
   * With this class, you can calc its fare.
   */
  class CRoute
  {
  private:
    station_id_t start;
    std::vector<std::pair<line_id_t, station_id_t> > way;
    boost::shared_ptr<CDatabase> db;
  public:
    /**
     * Constructor.
     * Constructor with existing CDatabase object.
     */
    CRoute(station_id_t station, CDatabase & db) : start(station), db(&db){}

    /**
     * Function to append a new part to the route.
     * @param[in] line    line id of route
     * @param[in] station station id of route
     */
    void append_route(line_id_t line, station_id_t station);

    /**
     * Function to calc fare of route.
     * When calculating fare, sometimes the route should be midified
     * because of some provisions. This function changes its route
     * while its execution.
     */
    int calc_fare_inplace();
  };
}
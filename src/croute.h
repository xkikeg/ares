#pragma once

#include <memory>
#include "ares.h"

namespace ares
{
  class CDatabase;

  /**
   * Class represents a route.
   * With this class, you can calc its fare.
   */
  class CRoute
  {
  private:
    std::vector<std::pair<line_id_t, station_id_t> > way;
    std::shared_ptr<CDatabase> db;
  public:
    /**
     * Constructor.
     * Constructor with existing CDatabase object.
     */
    CRoute(station_id_t station, std::shared_ptr<CDatabase> db)
      : db(db) {
      way.push_back(std::make_pair(0, station));
    }

    /**
     * Function to append a new part to the route.
     * @param[in] line    line id of route
     * @param[in] station station id of route
     */
    void append_route(line_id_t line, station_id_t station);

    /**
     * Function to validate route.
     */
    bool is_valid() const;

    /**
     * Function to calc fare of route.
     * When calculating fare, sometimes the route should be midified
     * because of some provisions. This function changes its route
     * while its execution.
     */
    int calc_fare_inplace();

    /**
     * Function to calc fare of Honshu main line from kilo.
     */
    static int calc_honshu_main(int kilo);
  };
}

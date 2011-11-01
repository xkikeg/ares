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
  public:
    struct CSegment
    {
      station_id_t begin, end;
      line_id_t line;

      CSegment(station_id_t begin)
        : begin(begin),
          end(0),
          line(0) {}

      CSegment(station_id_t begin,
               line_id_t line,
               station_id_t end)
        : begin(begin),
          end(end),
          line(line) {}

      /**
       * Check whether this segment is begin.
       */
      bool is_begin() {
        return line == 0 && end == 0;
      }
    };
  private:
    std::shared_ptr<CDatabase> db;
    std::vector<CSegment> way;
  public:
    /**
     * Constructor.
     * Constructor with existing CDatabase object.
     */
    CRoute(std::shared_ptr<CDatabase> db)
      : db(db) {}

    CRoute(std::shared_ptr<CDatabase> db, station_id_t begin)
      : db(db), way(1, CSegment(begin)) {}

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

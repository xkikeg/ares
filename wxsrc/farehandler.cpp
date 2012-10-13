#include "farehandler.h"

#include <cdatabase.h>
#include <croute.h>

IncrementFareHandler::IncrementFareHandler(std::shared_ptr<ares::CDatabase> db)
  : db(db), route(new ares::CRoute(db)) {}

void IncrementFareHandler::reset()
{
  route->init();
}

void IncrementFareHandler::init(ares::station_id_t begin,
                                ares::line_id_t line)
{
  route->init(begin);
  route->append_route(line, begin);
}

void IncrementFareHandler::changeStation(ares::station_id_t station)
{
  //! @TODO stub function
  /*
    実装の方針
    routeの末端の駅を変えるだけ。
   */
}

void IncrementFareHandler::changeLine(ares::line_id_t line)
{
  //! @TODO stub function
  /*
    実装の方針
    stationA -- line1 -- stationB -- line2 -- stationC
    において
    if stationB == stationC:
       まだ路線は伸びていない
       if line1 == line:
          line2--stationCを消す
       else:
          line2 <- line
    else:
       -- line -- stationCを追加
   */
}

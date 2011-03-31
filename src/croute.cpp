#include "croute.h"

namespace ares
{
  void CRoute::append_route(line_id_t line, station_id_t station)
  {
    way.push_back(std::pair<line_id_t, station_id_t>(line, station));
  }
}

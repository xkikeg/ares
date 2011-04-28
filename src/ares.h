#pragma once

#include <vector>
#include <utility>

namespace ares
{
  typedef int line_id_t;
  typedef int station_id_t;
  typedef int company_id_t;
  typedef std::vector<line_id_t> line_vector;
  typedef std::vector<station_id_t> station_vector;
  typedef std::pair<line_id_t, station_id_t> route_pair;
  typedef std::vector<route_pair> route_vector;
}

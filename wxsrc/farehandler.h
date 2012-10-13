#pragma once

#include <memory>
#include <ares.h>

namespace ares
{
  class CDatabase;
  class CRoute;
}

class IncrementFareHandler
{
private:
  std::shared_ptr<ares::CDatabase> db;
  std::unique_ptr<ares::CRoute> route;

public:
  IncrementFareHandler(std::shared_ptr<ares::CDatabase> db);
  ~IncrementFareHandler() = default;

  //! Reset all fare information.
  void reset();
  //! Initialize with specified line and station.
  void init(ares::station_id_t begin,
            ares::line_id_t line);
  //! Change current station keeping on the current line.
  void changeStation(ares::station_id_t station);
  //! Change current line on transit station.
  void changeLine(ares::line_id_t line);
};

#pragma once

#include "ckilo.h"

namespace ares
{
  struct CStation
  {
    station_id_t station_id;
    std::string name, yomi, denryaku;
    CHecto realkilo, fakekilo;

    CStation() = default;

    CStation(const station_id_t station_id,
             const std::string & name,
             const std::string & yomi,
             const std::string & denryaku,
             const int realkilo,
             const int fakekilo)
      : station_id(station_id),
        name(name),
        yomi(yomi),
        denryaku(denryaku),
        realkilo(CHecto(realkilo)),
        fakekilo(CHecto(fakekilo)) {}
  };

}

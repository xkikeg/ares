#pragma once

#include "ckilo.h"

namespace ares
{
  struct CStation
  {
    station_id_t id;
    std::string name, yomi, denryaku;
    CHecto realkilo, fakekilo;

    CStation() = default;

    template<class S>
    CStation(const station_id_t id,
             S && name,
             S && yomi,
             S && denryaku,
             const int realkilo,
             const int fakekilo)
      : id(id),
        name(std::forward<S>(name)),
        yomi(std::forward<S>(yomi)),
        denryaku(std::forward<S>(denryaku)),
        realkilo(CHecto(realkilo)),
        fakekilo(CHecto(fakekilo)) {}

    CStation(const CStation &) = default;
    CStation(CStation &&) = default;

    CStation & operator=(const CStation &) = default;
    CStation & operator=(CStation &&) = default;
  };
}

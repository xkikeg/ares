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
  typedef std::pair<line_id_t, station_id_t> station_fqdn_t;
  typedef std::vector<station_fqdn_t> connect_vector;

  /**
   * @~
   * 運賃体系上の会社の列挙体.
   */
  enum COMPANY_TYPE {
    COMPANY_HONSHU,
    COMPANY_HOKKAIDO,
    COMPANY_KYUSHU,
    COMPANY_SHIKOKU,
    MAX_COMPANY_TYPE,
  };

  /**
   * @~
   * 幹線と地方交通線の列挙体.
   */
  enum LINE_TYPE {
    LINE_LOCAL,
    LINE_MAIN,
    MAX_LINE_TYPE,
  };

  /**
   * @~
   * 実キロと擬制キロの列挙体.
   */
  enum KILO_TYPE {
    KILO_REAL,
    KILO_FAKE,
    MAX_KILO_TYPE,
  };
}

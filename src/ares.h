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
   * 運賃体系上のJR会社の列挙体.
   */
  enum JR_COMPANY_TYPE {
    JR_COMPANY_HONSHU,
    JR_COMPANY_HOKKAIDO,
    JR_COMPANY_KYUSHU,
    JR_COMPANY_SHIKOKU,
    MAX_JR_COMPANY_TYPE,
  };

  /**
   * @~
   * 運賃体系上の社線会社の列挙体.
   */
  enum COMPANY_TYPE {
    COMPANY_HONSHU   = JR_COMPANY_HONSHU,
    COMPANY_HOKKAIDO = JR_COMPANY_HOKKAIDO,
    COMPANY_KYUSHU   = JR_COMPANY_KYUSHU,
    COMPANY_SHIKOKU  = JR_COMPANY_SHIKOKU,
    COMPANY_KTR,
    COMPANY_OTHER,
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

  /**
   * @~
   * 電車特定区間の列挙体.
   */
  enum DENSHA_SPECIAL_TYPE {
    DENSHA_SPECIAL_NONE,
    DENSHA_SPECIAL_TOKYO,
    DENSHA_SPECIAL_OSAKA,
    DENSHA_SPECIAL_YAMANOTE,
    DENSHA_SPECIAL_OSAKAKANJO,
    MAX_DENSHA_SPECIAL_TYPE,
  };

  constexpr line_id_t INVALID_LINE_ID;
  constexpr station_id_t INVALID_STATION_ID;
}

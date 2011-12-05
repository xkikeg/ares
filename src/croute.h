#pragma once

#include <memory>
#include "ares.h"
#include "csegment.h"
#include "ckilo.h"

namespace ares
{
  class CDatabase;

  /**
   * @~english
   * Class represents a route.
   * With this class, you can calc its fare.
   */
  /**
   * @~japanese
   * 経路を表現するクラス.
   * 運賃計算や営業キロの取得の基本となるインターフェース.
   */
  class CRoute
  {
  private:
    std::shared_ptr<CDatabase> db;
    std::vector<CSegment> way;

  public:
    typedef std::vector<CSegment>::iterator iterator;
    typedef std::vector<CSegment>::const_iterator const_iterator;

    /**
     * Constructor.
     * Constructor with existing CDatabase object.
     */
    CRoute(std::shared_ptr<CDatabase> db)
      : db(db) {}

    CRoute(std::shared_ptr<CDatabase> db, station_id_t begin)
      : db(db), way(1, CSegment(begin)) {}

    friend std::ostream & operator<<(std::ostream & ost, const CRoute & route);

    bool operator==(const CRoute & b) const;

    iterator begin() { return way.begin(); }
    iterator end() { return way.end(); }
    const_iterator begin() const { return way.begin(); }
    const_iterator end() const{ return way.end(); }

    // /**
    //  * Set begin station. (does it fail with non-empty way?)
    //  */
    // bool set_begin(station_id_t station);

    /**
     * Function to append a new part to the route.
     * @param[in] line    line id of route
     * @param[in] station station id of route
     */
    void append_route(line_id_t line, station_id_t station);

    /**
     * @~english
     * Function to append a new part to the route.
     * @param[in] line    UTF-8 line name of route
     * @param[in] station UTF-8 station name of route
     */
    /**
     * @~japanese
     * 新しい経路の区間を加える関数.
     * @param[in] line    UTF-8でエンコードされた路線名
     * @param[in] station UTF-8でエンコードされた駅名
     */
    void append_route(const char * line, const char * station);

    /**
     * @~english
     * Function to append a new part to the route.
     * @param[in] line    line id of route
     * @param[in] begin   begin station id of route
     * @param[in] end     end station id of route
     */
    /**
     * @~japanese
     * 新しい経路の区間を加える関数.
     * @param[in] line    路線ID
     * @param[in] begin   始点の駅名ID
     * @param[in] end     終点の駅名ID
     */
    void append_route(line_id_t line, station_id_t begin, station_id_t end);

    /**
     * @~english
     * Function to append a new part to the route.
     * @param[in] line    UTF-8 line name of route
     * @param[in] begin   UTF-8 begin station name of route
     * @param[in] end     UTF-8 end station name of route
     */
    /**
     * @~japanese
     * 新しい経路の区間を加える関数.
     * @param[in] line    UTF-8でエンコードされた路線名
     * @param[in] begin   UTF-8でエンコードされた始点の駅名
     * @param[in] end     UTF-8でエンコードされた終点の駅名
     */
    void append_route(const char * line, const char * begin, const char * end);

    /**
     * Function to check contains.
     */
    bool is_contains(station_id_t station) const;

    /**
     * Function to validate route.
     */
    bool is_valid() const;

    //! Return kilo sumulation of the route.
    CKilo get_kilo() const;

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

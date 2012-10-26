#pragma once

#include <string>
#include <memory>
#include <stdexcept>
#include <boost/utility.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include "ares.h"

namespace sqlite3_wrapper
{
  class SQLite;
  class IOException;
}

namespace ares
{
  enum find_mode {
    FIND_PARTIAL,
    FIND_PREFIX,
    FIND_SUFFIX,
    FIND_EXACT,
  };

  using sqlite3_wrapper::SQLite;
  typedef sqlite3_wrapper::IOException IOException;

  class CSegment;
  class CKiloValue;
  class CStation;

  /**
   * @~english
   * Exception to represent that no object has found.
   */
  /**
   * @~japanese
   * オブジェクトが見つからなかった時の例外.
   */
  class DoesNotExist : public std::logic_error
  {
  public:
    explicit DoesNotExist(const std::string & query)
      : std::logic_error("Object does not exist: " + query) {}
  };

  /**
   * Exception to represent that multiple object returned though
   * only one object is required.
   */
  class MultipleObjectReturned : public std::logic_error
  {
  public:
    /**
     * Constructor.
     * @param[in] query      Query string that raise multiple object error.
     * @param[in] num        The number of result returned to query.
     */
    MultipleObjectReturned(const char * query, size_t num)
      : std::logic_error(std::string("query '") + query + "' returned"
                         + boost::lexical_cast<std::string>(num) + " objects.") {}
  };

  /**
   * Database object of ares wrapping sqlite3 object.
   * With this object, you can search station or line name,
   * get connections of lines and so on.
   */
  class CDatabase : boost::noncopyable
  {
  private:
    std::unique_ptr<SQLite> db;

  public:
    /**
     * Constructor.
     * Construct a CDatabase object with database filename.
     * @param[in] dbname The filename of SQLite database.
     */
    CDatabase(const char * dbname, bool memcache=true);

    /**
     * Convert function from line id to name.
     * @param[in] line The desired line id.
     * @return The line name corresponding specified line id.
     */
    std::string get_line_name(line_id_t line) const;

    /**
     * Convert function from station id to name.
     * @param[in] station The desired station id.
     * @return The station name corresponding specified station id.
     */
    std::string get_station_name(station_id_t station) const;

    /**
     * Convert function from station id to yomi.
     * @param[in] station The desired station id.
     * @return The station yomi corresponding specified station id.
     */
    std::string get_station_yomi(station_id_t station) const;

    /**
     * Convert function from station id to denryaku.
     * @param[in] station The desired station id.
     * @return The station denryaku corresponding specified station id.
     */
    std::string get_station_denryaku(station_id_t station) const;

    /**
     * すべての路線名を返す.
     * @param[in] result 路線IDとUTF-8エンコードされた駅名.
     */
    void get_all_lines_name(std::vector<std::pair<
                            line_id_t, std::string> > &result) const;

    /**
     * 指定された路線の駅の集合を返す.
     * @param[in]  line   結果を得たい路線ID.
     * @param[out] result 結果を格納する配列.
     */
    void get_stations_of_line(line_id_t line,
                              std::vector<CStation> &result) const;

    /**
     * 区間を限定して指定された路線の駅の集合を返す.
     * @param[in]  line   結果を得たい路線ID.
     * @param[in]  begin  始点の駅ID.
     * @param[in]  end    終点の駅ID.
     * @param[out] result 結果を格納する配列.
     */
    size_t get_stations_of_segment(line_id_t line,
                                   station_id_t begin,
                                   station_id_t end,
                                   station_vector & result) const;

    /**
     * 指定された駅の所属する路線の集合を返す.
     * @param[in]  station 結果を得たい駅ID.
     * @param[out] result  結果を格納する配列.
     */
    void get_lines_of_station(station_id_t station,
                              line_vector & result) const;

    /**
     * Find lines' id from line name.
     * @param[in]  name Specify string to find.
     * @param[in]  mode Specify searching mode.
     * @param[out] list vector to add found line id's.
     */
    void find_lineid(const char * name,
                     const find_mode mode,
                     line_vector & list) const;

    //! Find lines from line name with kanji.
    void find_lineid_with_name(const char * name,
                               const find_mode mode,
                               line_vector & list) const;
    //! Find lines from line name with yomi.
    void find_lineid_with_yomi(const char * name,
                               const find_mode mode,
                               line_vector & list) const;
    //! Find lines from line name with alias.
    void find_lineid_with_alias(const char * name,
                                const find_mode mode,
                                line_vector & list) const;

    /**
     * Get the line's id from line name.
     * @param[in]  name Specify string to get.
     * @param[in]  mode Specify searching mode.
     * @return line id if exists only one, otherwise zero.
     */
    line_id_t get_lineid(const char * name,
                         const find_mode mode = FIND_EXACT) const;

    //! Find stations' id from station name.
    void find_stationid(const char * name,
                        const find_mode mode,
                        station_vector & list) const;
    //! Find stations' id from station name with kanji.
    void find_stationid_with_name(const char * name,
                                  const find_mode mode,
                                  station_vector & list) const;
    //! Find stations' id from station name with yomi.
    void find_stationid_with_yomi(const char * name,
                                  const find_mode mode,
                                  station_vector & list) const;
    //! Find stations' id from station name with denryaku.
    void find_stationid_with_denryaku(const char * name,
                                      const find_mode mode,
                                      station_vector & list) const;

    /**
     * Get the station's id from station name.
     * @param[in] name Specify string to get.
     * @param[in] mode Specify searching mode.
     * @return station id if exists only one station, otherwise zero.
     */
    station_id_t get_stationid(const char * name,
                               const find_mode mode = FIND_EXACT) const;

    //! Get lines' id connecting with.
    void get_connect_line(line_id_t line,
                          connect_vector & list) const;

    /**
     * ある駅の所属する路線IDを取得する.
     * @param[in]  station 指定する駅ID.
     * @param[out] result  結果の路線IDの配列.
     */
    void get_belong_line(station_id_t station,
                          line_vector & result) const;

    //! Check if the station is in the line.
    bool is_belong_to_line(line_id_t line, station_id_t station) const;

    //! Check station is in the segment.
    bool is_contains(const CSegment & range, const station_id_t station) const;

    //! Get company id from company name.
    company_id_t get_company_id(const char * name) const;

    //! 会社名をIDから取得する.
    std::string get_company_name(const company_id_t id) const;

    //! Get fare value from table.
    int get_fare_table(const char * table,
                       company_id_t company,
                       int kilo) const;

    //! JR四国・九州の地方交通線特例運賃表を引く
    boost::optional<int> get_fare_country_table(const char * table,
                                                company_id_t company,
                                                int realkilo,
                                                int fakekilo) const;

    //! 加算運賃や社線運賃の表を引く
    boost::optional<
      std::pair<bool,int> > get_special_fare(line_id_t line,
                                             station_id_t begin,
                                             station_id_t end) const;

    /**
     * @~
     * 電車特定区間を調べる.
     * @param[in] line  路線ID.
     * @param[in] range 営業キロの範囲.
     * @return          指定された区間の電車特定区間IDと山手環状特例ID.
     *                  冗長だが一般性のため両方返している.
     */
    std::pair<DENSHA_SPECIAL_TYPE, DENSHA_SPECIAL_TYPE>
    get_denshaid(const line_id_t line,
                 const std::pair<int, int> range) const;

    //! Get kilo from line id & station id.
    int get_kilo(const line_id_t line,
                 const station_id_t station) const;

    /**
     * @~english
     * Get kilo range of [begin, end] in line.
     */
    /**
     * @~japanese
     * 指定された区間のキロ程を返す.
     * @param[in] line  路線ID.
     * @param[in] begin 始点の駅ID.
     * @param[in] end   終点の駅ID.
     * @return キロ程の小さい値と大きい値のペア.
     */
    std::pair<int, int> get_range(const line_id_t line,
                                  const station_id_t begin,
                                  const station_id_t end) const;

    //! Get company id & 10*kilo.
    /**
     * @param[in]     line     路線ID
     * @param[in]     begin    始点の駅ID
     * @param[in]     end      終点の駅ID
     * @param[in,out] result   結果の配列
     * @param[out]    is_main  幹線ならtrue, 地方交通線ならfalse
     * @param[out]    denshaid 電車特定区間ID
     * @param[out]    circleid 山手・大阪環状特例区間ID
     */
    bool get_company_and_kilo(const line_id_t line,
                              const station_id_t begin,
                              const station_id_t end,
                              std::vector<CKiloValue> & result,
                              bool & is_main,
                              DENSHA_SPECIAL_TYPE & denshaid,
                              DENSHA_SPECIAL_TYPE & circleid) const;
  };
}

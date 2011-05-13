#pragma once

#include <string>
#include <memory>
#include <stdexcept>
#include <boost/utility.hpp>
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

  /**
   * Exception to represent that no object has found.
   */
  class DoesNotExist : public std::logic_error
  {
  public:
    DoesNotExist() : std::logic_error("Object does not exist.") {}
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
    CDatabase(const char * dbname);

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
     * Find lines' id from line name.
     * @param[in]  name Specify string to find.
     * @param[in]  mode Specify searching mode.
     * @param[out] list vector to add found line id's.
     */
    bool find_lineid(const char * name,
                     const find_mode mode,
                     line_vector & list) const;

    //! Find lines from line name with kanji.
    bool find_lineid_with_name(const char * name,
                               const find_mode mode,
                               line_vector & list) const;
    //! Find lines from line name with yomi.
    bool find_lineid_with_yomi(const char * name,
                               const find_mode mode,
                               line_vector & list) const;
    //! Find lines from line name with alias.
    bool find_lineid_with_alias(const char * name,
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
    bool find_stationid(const char * name,
                        const find_mode mode,
                        station_vector & list) const;
    //! Find stations' id from station name with kanji.
    bool find_stationid_with_name(const char * name,
                                  const find_mode mode,
                                  station_vector & list) const;
    //! Find stations' id from station name with yomi.
    bool find_stationid_with_yomi(const char * name,
                                  const find_mode mode,
                                  station_vector & list) const;
    //! Find stations' id from station name with denryaku.
    bool find_stationid_with_denryaku(const char * name,
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

    //! Find lines' id connecting with.
    bool find_connect_line(line_id_t line,
                           connect_vector & list) const;

    //! Get company id from company name.
    company_id_t get_company_id(const char * name) const;

    //! Get fare value from table.
    int get_fare_table(const char * table,
                       company_id_t company,
                       int kilo) const;

    //! Check if the station is in the line.
    bool is_belong_to_line(line_id_t line, station_id_t station) const;
  };
}

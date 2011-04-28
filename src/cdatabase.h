#pragma once

#include <string>
#include <memory>
#include <boost/utility.hpp>
#include "sqlite3_wrapper.h"
#include "ares.h"

namespace ares
{
  enum search_mode {
    SEARCH_PARTIAL,
    SEARCH_PREFIX,
    SEARCH_SUFFIX,
    SEARCH_EXACT,
  };

  /**
   * Database object of ares wrapping sqlite3 object.
   * With this object, you can search station or line name,
   * get connections of lines and so on.
   */
  using sqlite3_wrapper::SQLite;
  typedef sqlite3_wrapper::IOException IOException;
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
    CDatabase(const char * dbname) : db(new SQLite(dbname, SQLITE_OPEN_READONLY)){}

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
     * Seach lines from line name.
     * @param[in]  name Specify string to find.
     * @param[in]  mode Specify searching mode.
     * @param[out] list vector to add found line id's.
     */
    bool search_line(const char * name,
		     const search_mode mode,
		     line_vector & list) const;

    //! Seach lines from line name with kanji.
    bool search_line_with_name(const char * name,
			       const search_mode mode,
			       line_vector & list) const;
    //! Seach lines from line name with yomi.
    bool search_line_with_yomi(const char * name,
			       const search_mode mode,
			       line_vector & list) const;
    //! Seach lines from line name with alias.
    bool search_line_with_alias(const char * name,
				const search_mode mode,
				line_vector & list) const;

    //! Search stations from station name.
    bool search_station(const char * name,
			const search_mode mode,
			station_vector & list) const;
    //! Seach stations from station name with kanji.
    bool search_station_with_name(const char * name,
				  const search_mode mode,
				  station_vector & list) const;
    //! Seach stations from station name with yomi.
    bool search_station_with_yomi(const char * name,
				  const search_mode mode,
				  station_vector & list) const;
    //! Seach stations from station name with denryaku.
    bool search_station_with_denryaku(const char * name,
				      const search_mode mode,
				      station_vector & list) const;
    //! Search lines connecting with.
    bool search_connect_line(line_id_t line,
			     route_vector & list) const;

    //! Get company id from company name.
    company_id_t get_company_id(const char * name);

    //! Get fare value from table.
    int get_fare_table(const char * table,
		       company_id_t company,
		       int kilo);
  };
}

#include <string>
#include <exception>
#include <boost/utility.hpp>
#include <sqlite3.h>
#include "ares.h"

namespace ares
{
  class IOException : public std::exception
  {
  };

  enum SearchMode {
    SEARCH_PREFIX,
    SEARCH_SUFFIX,
    SEARCH_EXACT,
  };

  /**
   * Database object of ares wrapping sqlite3 object.
   * With this object, you can search station or line name,
   * get connections of lines and so on.
   */
  class CDatabase : boost::noncopyable
  {
  private:
    sqlite3 *db;
  public:
    /**
     * Constructor.
     * Construct a CDatabase object with database filename.
     * @param[in] dbname The filename of SQLite database.
     */
    CDatabase(const char * dbname);

    /// Destructor
    ~CDatabase();

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
     * Convert function from a list of line id to the one of name.
     * @param[in]  lines The desired line ids.
     * @param[out] names The line names corresponding specified line ids.
     */
    int get_line_name(const line_vector & lines,
		      std::vector<std::string> & names) const;

    /**
     * Convert function from a list of station id to the one of name.
     * @param[in]  stations The desired station ids.
     * @param[out] names    The station names corresponding specified station ids.
     */
    int get_station_name(const station_vector & stations,
			 std::vector<std::string> & names) const;

    /**
     * Seach lines from line name.
     * @param[in]  name Specify string to find.
     * @param[in]  mode Specify searching mode.
     * @param[out] list vector to add found line id's.
     */
    bool search_line(const char * name,
		     const SearchMode mode,
		     line_vector & list) const;

    //! Seach lines from line name with kanji.
    bool search_line_with_name(const char * name,
			       const SearchMode mode,
			       line_vector & list) const;
    //! Seach lines from line name with yomi.
    bool search_line_with_yomi(const char * name,
			       const SearchMode mode,
			       line_vector & list) const;
    //! Seach lines from line name with denryaku.
    bool search_line_with_denryaku(const char * name,
				   const SearchMode mode,
				   line_vector & list) const;
    //! Search stations from station name.
    bool search_station(const char * name,
			const SearchMode mode,
			station_vector & list) const;
    //! Seach stations from station name with kanji.
    bool search_station_with_name(const char * name,
			       const SearchMode mode,
			       station_vector & list) const;
    //! Seach stations from station name with yomi.
    bool search_station_with_yomi(const char * name,
			       const SearchMode mode,
			       station_vector & list) const;
    //! Seach stations from station name with denryaku.
    bool search_station_with_denryaku(const char * name,
				   const SearchMode mode,
				   station_vector & list) const;
    //! Search lines connecting with.
    int search_connect_line(line_id_t line,
			    line_vector & list) const;
  };
}

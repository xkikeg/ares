/* -*-coding: utf-8-*- */
#include <iostream>
#include <cstdlib>
#include <cstring>
#include "cdatabase.h"

namespace ares
{
  namespace
  {
    int callback_pushstation(void * _array, int argc, char ** argv, char ** column)
    {
      station_vector * array = static_cast<station_vector*>(_array);
      array->push_back(std::atoi(argv[0]));
      return 0;
    }
  }

  std::string CDatabase::get_line_name(line_id_t line) const
  {
    return "";
  }

  std::string CDatabase::get_station_name(station_id_t station) const
  {
    return "";
  }

  int CDatabase::get_line_name(const line_vector & lines,
			       std::vector<std::string> & names) const
  {
    return 0;
  }

  int CDatabase::get_station_name(const station_vector & stations,
				  std::vector<std::string> & names) const
  {
    return 0;
  }

  bool CDatabase::search_line(const char * name,
			      const SearchMode mode,
			      line_vector & list) const
  {
    return false;
  }

  bool CDatabase::search_line_with_name(const char * name,
					const SearchMode mode,
					line_vector & list) const
  {
    return false;
  }

  bool CDatabase::search_line_with_yomi(const char * name,
					const SearchMode mode,
					line_vector & list) const
  {
    return false;
  }

  bool CDatabase::search_line_with_denryaku(const char * name,
					    const SearchMode mode,
					    line_vector & list) const
  {
    return false;
  }

  bool CDatabase::search_station(const char * name,
				 const SearchMode mode,
				 station_vector & list) const
  {
    return false;
  }

  bool CDatabase::search_station_with_name(const char * name,
					   const SearchMode mode,
					   station_vector & list) const
  {
    // should mention about kakko station name
    // SELECT id FROM station WHERE
    //        name LIKE 'name%' OR name LIKE '（%）name%'
    //        OR name LIKE '%name%' (if not forward)
    //        ;
    // SQL Injection can cause.
    int rc;
    char * query = sqlite3_mprintf("SELECT id FROM station WHERE name LIKE"
				   "'%s%%' OR name LIKE '（%%）%s%%';",
				   name, name);
    rc = sqlite3_exec(db->ptr(), query, callback_pushstation, &list, NULL);
    if(rc != SQLITE_OK)
      {
	std::cerr << db->errmsg() << std::endl;
	return false;
      }
    sqlite3_free(query);
    return true;
  }

  bool CDatabase::search_station_with_yomi(const char * name,
					   const SearchMode mode,
					   station_vector & list) const
  {
    // SELECT id FROM station WHERE yomi LIKE 'name%';
    int rc;
    std::string name_ = name;
    name_ += "%";
    const char * sql = "SELECT id FROM station WHERE yomi LIKE ?;";
    sqlite3_wrapper::SQLiteStmt station_yomi_stmt(*db, sql, std::strlen(sql));
    station_yomi_stmt.reset();
    station_yomi_stmt.bind(1, name_);
    while((rc = station_yomi_stmt.step()) == SQLITE_ROW)
      {
	list.push_back(station_yomi_stmt.column(0));
      }
    if (rc != SQLITE_DONE)
      std::cerr << db->errmsg() << std::endl;
  }

  bool CDatabase::search_station_with_denryaku(const char * name,
					       const SearchMode mode,
					       station_vector & list) const
  {
    return false;
  }

  int CDatabase::search_connect_line(line_id_t line,
				     line_vector & list) const
  {
    return 0;
  }

}

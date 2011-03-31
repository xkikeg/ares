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

  CDatabase::CDatabase(const char * dbname)
  {
    int rc;
    rc = sqlite3_open_v2(dbname, &db, SQLITE_OPEN_READONLY, NULL);
    if(rc != SQLITE_OK)
      {
	const char * errmsg = sqlite3_errmsg(db);
	std::cerr << errmsg << std::endl;
	sqlite3_close(db);
	throw IOException();
      }
  }

  CDatabase::~CDatabase()
  {
    sqlite3_close(db);
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
    int ret;
    char * query = sqlite3_mprintf("SELECT id FROM station WHERE name LIKE"
				   "'%s%%' OR name LIKE '（%%）%s%%';",
				   name, name);
    ret = sqlite3_exec(db, query, callback_pushstation, &list, NULL);
    if(ret != SQLITE_OK)
      {
	std::cerr << sqlite3_errmsg(db) << std::endl;
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
    int ret;
    sqlite3_stmt * station_yomi_stmt = NULL;
    std::string name_ = name;
    name_ += "%";
    const char * sql = "SELECT id FROM station WHERE yomi LIKE ?;";
    sqlite3_prepare(db, sql, std::strlen(sql), &station_yomi_stmt, NULL);
    sqlite3_reset(station_yomi_stmt);
    char * name__ = sqlite3_mprintf("%Q", name_.c_str());
    sqlite3_bind_text(station_yomi_stmt, 1, name__, std::strlen(name__), sqlite3_free);
    while((ret = sqlite3_step(station_yomi_stmt)) == SQLITE_ROW)
      {
	list.push_back(sqlite3_column_int(station_yomi_stmt, 0));
      }
    if (ret != SQLITE_DONE)
      std::cerr << sqlite3_errmsg(db) << std::endl;
    sqlite3_finalize(station_yomi_stmt);
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

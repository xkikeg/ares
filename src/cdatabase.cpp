/* -*-coding: utf-8-*- */
#include <iostream>
#include <cstdlib>
#include <cstring>
#include "cdatabase.h"
#include "util.hpp"

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

    bool add_percent(std::string & str, const search_mode mode)
    {
      switch(mode)
	{
	case SEARCH_PARTIAL:
	  str = "%" + str + "%";
	  break;

	case SEARCH_PREFIX:
	  str += "%";
	  break;

	case SEARCH_SUFFIX:
	  str = "%" + str;
	  break;

	case SEARCH_EXACT:
	  break;

	default:
	  return false;
	}
      return true;
    }
  }

  std::string CDatabase::get_line_name(line_id_t line) const
  {
    return "";
  }

  std::string CDatabase::get_station_name(station_id_t station) const
  {
    const char * sql = "SELECT name FROM station WHERE id = ?";
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.reset();
    stmt.bind(1, station);
    int rc = stmt.step();
    if(rc != SQLITE_ROW && rc != SQLITE_DONE)
      {
	std::cerr << "such a station id not found: " << station << std::endl;
	return "";
      }
    const char * name = stmt.column(0);
    return std::string(name);
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
			      const search_mode mode,
			      line_vector & list) const
  {
    return false;
  }

  bool CDatabase::search_line_with_name(const char * name,
					const search_mode mode,
					line_vector & list) const
  {
    return false;
  }

  bool CDatabase::search_line_with_yomi(const char * name,
					const search_mode mode,
					line_vector & list) const
  {
    return false;
  }

  bool CDatabase::search_line_with_denryaku(const char * name,
					    const search_mode mode,
					    line_vector & list) const
  {
    return false;
  }

  bool CDatabase::search_station(const char * name,
				 const search_mode mode,
				 station_vector & list) const
  {
    return false;
  }

  bool CDatabase::search_station_with_name(const char * name,
					   const search_mode mode,
					   station_vector & list) const
  {
    const char * sql = "SELECT id FROM station WHERE name LIKE ? OR name LIKE ?";
    std::string name_norm(name);
    std::string name_paren("（%）");
    if(!add_percent(name_norm, mode))
      return false;
    name_paren += name_norm;
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.reset();
    stmt.bind(1, name_norm);
    stmt.bind(2, name_paren);
    int rc;
    while((rc = stmt.step()) == SQLITE_ROW)
      {
	list.push_back(stmt.column(0));
      }
    if(rc != SQLITE_DONE)
      {
	std::cerr << db->errmsg() << std::endl;
	return false;
      }
    return true;
  }

  bool CDatabase::search_station_with_yomi(const char * name,
					   const search_mode mode,
					   station_vector & list) const
  {
    // SELECT id FROM station WHERE yomi LIKE 'name%';
    int rc;
    std::string name_(name);
    add_percent(name_, mode);
    const char * sql = "SELECT id FROM station WHERE yomi LIKE ?;";
    sqlite3_wrapper::SQLiteStmt station_yomi_stmt(*db, sql, std::strlen(sql));
    station_yomi_stmt.reset();
    station_yomi_stmt.bind(1, name_);
    while((rc = station_yomi_stmt.step()) == SQLITE_ROW)
      {
	list.push_back(station_yomi_stmt.column(0));
      }
    if (rc != SQLITE_DONE)
      {
	std::cerr << db->errmsg() << std::endl;
	return false;
      }
    return true;
  }

  bool CDatabase::search_station_with_denryaku(const char * name,
					       const search_mode mode,
					       station_vector & list) const
  {
    // SELECT id FROM station WHERE denryaku LIKE 'name%' OR denryaku LIKE '__name%';
    int rc;
    std::string name_(name);
    add_percent(name_, mode);
    const char * sql = "SELECT id FROM station WHERE denryaku LIKE ?;";
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql, std::strlen(sql));
    std::cerr << "name:" << name << ",name_:" << name_ << std::endl;
    std::wstring wname_;
    liquid::multi2wide(name_, wname_);
    if(wname_.length() <= 2)
      {
	std::string name_onlystation("__");
	name_onlystation += name_;
	stmt.reset();
	stmt.bind(1, name_onlystation);
	while((rc = stmt.step()) == SQLITE_ROW)
	  {
	    list.push_back(stmt.column(0));
	  }
	if (rc != SQLITE_DONE)
	  {
	    std::cerr << db->errmsg() << std::endl;
	    return false;
	  }
      }
    stmt.reset();
    stmt.bind(1, name_);
    while((rc = stmt.step()) == SQLITE_ROW)
      {
	list.push_back(stmt.column(0));
      }
    if (rc != SQLITE_DONE)
      {
	std::cerr << db->errmsg() << std::endl;
	return false;
      }
    return true;
  }

  int CDatabase::search_connect_line(line_id_t line,
				     line_vector & list) const
  {
    return 0;
  }

}

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
    bool add_percent(std::string & str, const find_mode mode)
    {
      switch(mode)
      {
      case FIND_PARTIAL:
        str = "%" + str + "%";
        break;

      case FIND_PREFIX:
        str += "%";
        break;

      case FIND_SUFFIX:
        str = "%" + str;
        break;

      case FIND_EXACT:
        break;

      default:
        return false;
      }
      return true;
    }
  }

  std::string CDatabase::get_line_name(line_id_t line) const
  {
    const char * sql = "SELECT linename FROM line WHERE lineid = ?";
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, line);
    int rc = stmt.step();
    if(rc != SQLITE_ROW && rc != SQLITE_DONE)
    {
      std::cerr << "such a line id not found: " << line << std::endl;
      return "";
    }
    const char * name = stmt.column(0);
    return std::string(name);
  }

  std::string CDatabase::get_station_name(station_id_t station) const
  {
    const char * sql = "SELECT stationname FROM station WHERE stationid = ?";
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql, std::strlen(sql));
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

  std::string CDatabase::get_station_yomi(station_id_t station) const
  {
    const char * sql = "SELECT stationyomi FROM station WHERE stationid = ?";
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, station);
    int rc = stmt.step();
    if(rc != SQLITE_ROW && rc != SQLITE_DONE)
    {
      std::cerr << "such a station id not found: " << station << std::endl;
      return "";
    }
    const char * yomi = stmt.column(0);
    return std::string(yomi);
  }

  std::string CDatabase::get_station_denryaku(station_id_t station) const
  {
    const char * sql = "SELECT stationdenryaku FROM station WHERE stationid = ?";
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, station);
    int rc = stmt.step();
    if(rc != SQLITE_ROW && rc != SQLITE_DONE)
    {
      std::cerr << "such a station id not found: " << station << std::endl;
      return "";
    }
    const char * denryaku = stmt.column(0);
    return std::string(denryaku);
  }

  bool CDatabase::find_lineid(const char * name,
                              const find_mode mode,
                              line_vector & list) const
  {
    bool ret_name = this->find_lineid_with_name(name, mode, list);
    bool ret_yomi = this->find_lineid_with_yomi(name, mode, list);
    bool ret_alias = this->find_lineid_with_alias(name, mode, list);
    return (ret_name & ret_yomi & ret_alias);
  }

  bool CDatabase::find_lineid_with_name(const char * name,
                                        const find_mode mode,
                                        line_vector & list) const
  {
    std::string name_(name);
    add_percent(name_, mode);
    const char * sql = "SELECT lineid FROM line WHERE linename LIKE ?;";
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, name_);
    return stmt.fill_column(list, 0);
  }

  bool CDatabase::find_lineid_with_yomi(const char * name,
                                        const find_mode mode,
                                        line_vector & list) const
  {
    std::string name_(name);
    add_percent(name_, mode);
    const char * sql = "SELECT lineid FROM line WHERE lineyomi LIKE ?;";
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, name_);
    return stmt.fill_column(list, 0);
  }

  bool CDatabase::find_lineid_with_alias(const char * name,
                                         const find_mode mode,
                                         line_vector & list) const
  {
    return true;
  }

  line_id_t CDatabase::get_lineid(const char * name,
                                  const find_mode mode) const
  {
    line_vector v;
    find_lineid(name, mode, v);
    if(v.empty())
    {
      throw DoesNotExist();
    }
    if(v.size() > 1)
    {
      throw MultipleObjectReturned(name, v.size());
    }
    return v[0];
  }

  bool CDatabase::find_stationid(const char * name,
                                 const find_mode mode,
                                 station_vector & list) const
  {
    bool ret_name = this->find_stationid_with_name(name, mode, list);
    bool ret_yomi = this->find_stationid_with_yomi(name, mode, list);
    bool ret_denryaku = this->find_stationid_with_denryaku(name, mode, list);
    return (ret_name & ret_yomi & ret_denryaku);
  }

  bool CDatabase::find_stationid_with_name(const char * name,
                                           const find_mode mode,
                                           station_vector & list) const
  {
    const char * sql = "SELECT stationid FROM station WHERE stationname LIKE ? OR stationname LIKE ?";
    std::string name_norm(name);
    std::string name_paren("（%）");
    if(!add_percent(name_norm, mode))
      return false;
    name_paren += name_norm;
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, name_norm);
    stmt.bind(2, name_paren);
    return stmt.fill_column(list, 0);
  }

  bool CDatabase::find_stationid_with_yomi(const char * name,
                                           const find_mode mode,
                                           station_vector & list) const
  {
    // SELECT id FROM station WHERE yomi LIKE 'name%';
    std::string name_(name);
    add_percent(name_, mode);
    const char * sql = "SELECT stationid FROM station WHERE stationyomi LIKE ?;";
    sqlite3_wrapper::SQLiteStmt station_yomi_stmt(*db, sql, std::strlen(sql));
    station_yomi_stmt.bind(1, name_);
    return station_yomi_stmt.fill_column(list, 0);
  }

  bool CDatabase::find_stationid_with_denryaku(const char * name,
                                               const find_mode mode,
                                               station_vector & list) const
  {
    // SELECT id FROM station WHERE denryaku LIKE 'name%' OR denryaku LIKE '__name%';
    std::string name_(name);
    add_percent(name_, mode);
    const char * sql = "SELECT stationid FROM station WHERE stationdenryaku LIKE ?;";
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql, std::strlen(sql));
    std::wstring wname_;
    liquid::multi2wide(name_, wname_);
    if(wname_.length() <= 2)
    {
      std::string name_onlystation("__");
      name_onlystation += name_;
      stmt.bind(1, name_onlystation);
      bool ret = stmt.fill_column(list, 0);
      if(!ret) return ret;
      stmt.reset();
    }
    stmt.bind(1, name_);
    return stmt.fill_column(list, 0);
  }

  station_id_t CDatabase::get_stationid(const char * name,
                                        const find_mode mode) const
  {
    station_vector v;
    find_stationid(name, mode, v);
    if(v.empty())
    {
      throw DoesNotExist();
    }
    if(v.size() > 1)
    {
      throw MultipleObjectReturned(name, v.size());
    }
    return v[0];
  }

  bool CDatabase::find_connect_line(line_id_t line,
                                    route_vector & list) const
  {
    const char * sql =
      "SELECT kilo.lineid, kilo.stationid FROM kilo NATURAL JOIN station"
      " WHERE kilo.stationid IN ("
      "  SELECT kilo.stationid FROM kilo WHERE lineid = ?1"
      " ) AND kilo.lineid != ?1";
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, line);
    return stmt.fill_column(list, 0, 1);
  }

  company_id_t CDatabase::get_company_id(const char * name)
  {
    const char * sql =
      "SELECT companyid FROM company WHERE companyname LIKE ?";
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, name);
    int rc = stmt.step();
    if (rc == SQLITE_ROW)
    {
      return stmt.column(0);
    }
    if(rc != SQLITE_DONE)
    {
      std::cerr << db->errmsg() << std::endl;
    }
    return -1;
  }

  int CDatabase::get_fare_table(const char * table,
                                company_id_t company,
                                int kilo)
  {
    const char * sql =
      "SELECT fare.fare FROM fare WHERE type = ?1 AND companyid = ?2"
      " AND minkilo <= ?3"
      " AND maxkilo >= ?3";
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, table);
    stmt.bind(2, company);
    stmt.bind(3, kilo);
    int rc = stmt.step();
    if (rc == SQLITE_ROW)
      return stmt.column(0);
    if(rc != SQLITE_DONE)
      std::cerr << db->errmsg() << std::endl;
    return -1;
  }
}

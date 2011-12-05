/* -*-coding: utf-8-*- */
#include <iostream>
#include <cstdlib>
#include <cstring>

#include "util.hpp"
#include "sqlite3_wrapper.h"
#include "cdatabase.h"
#include "csegment.h"
#include "ckilo.h"

namespace ares
{
  namespace
  {
    std::string add_percent(std::string str, const find_mode mode)
    {
      switch(mode)
      {
      case FIND_PARTIAL: return std::move("%" + str + "%");

      case FIND_PREFIX: return std::move(str + "%");

      case FIND_SUFFIX: return std::move("%" + str);

      case FIND_EXACT: return std::move(str);

      default: return str;
      }
    }
  }

  CDatabase::CDatabase(const char * dbname)
  : db(new SQLite(dbname, SQLITE_OPEN_READONLY))
  {
  }

  std::string CDatabase::get_line_name(line_id_t line) const
  {
    const char sql[] = "SELECT linename FROM line WHERE lineid = ?";
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
    const char sql[] = "SELECT stationname FROM station WHERE stationid = ?";
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
    const char sql[] = "SELECT stationyomi FROM station WHERE stationid = ?";
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
    const char sql[] = "SELECT stationdenryaku FROM station WHERE stationid = ?";
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
    name_ = add_percent(std::move(name_), mode);
    const char sql[] = "SELECT lineid FROM line WHERE linename LIKE ?;";
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, name_);
    return stmt.fill_column(list, 0);
  }

  bool CDatabase::find_lineid_with_yomi(const char * name,
                                        const find_mode mode,
                                        line_vector & list) const
  {
    std::string name_(name);
    name_ = add_percent(std::move(name_), mode);
    const char sql[] = "SELECT lineid FROM line WHERE lineyomi LIKE ?;";
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
    const char sql[] = "SELECT stationid FROM station WHERE stationname LIKE ? OR stationname LIKE ?";
    std::string name_norm(name);
    std::string name_paren("（%）");
    name_norm = add_percent(std::move(name_norm), mode);
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
    name_ = add_percent(std::move(name_), mode);
    const char sql[] = "SELECT stationid FROM station WHERE stationyomi LIKE ?;";
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
    name_ = add_percent(std::move(name_), mode);
    const char sql[] = "SELECT stationid FROM station WHERE stationdenryaku LIKE ?;";
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
                                    connect_vector & list) const
  {
    const char sql[] =
      "SELECT kilo.lineid, kilo.stationid FROM kilo NATURAL JOIN station"
      " WHERE kilo.stationid IN ("
      "  SELECT kilo.stationid FROM kilo WHERE lineid = ?1"
      " ) AND kilo.lineid != ?1";
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, line);
    return stmt.fill_column(list, 0, 1);
  }

  company_id_t CDatabase::get_company_id(const char * name) const
  {
    const char sql[] =
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
                                int kilo) const
  {
    const char sql[] =
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

  int CDatabase::get_kilo(const line_id_t line,
                          const station_id_t station) const
  {
    const char sql[] =
      "SELECT kilo FROM kilo WHERE lineid=? AND stationid=?";
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, line);
    stmt.bind(2, station);
    int rc = stmt.step();
    if (rc == SQLITE_ROW)
      return stmt.column(0);
    // DB error check
    if(rc != SQLITE_DONE)
      std::cerr << db->errmsg() << std::endl;
    // DB or argument error handling
    return -1;
  }

  /**
   * @note この実装は1路線にたかだか2会社しか入らないことを暗黙の仮定にしている。
   * 更に言うと、2会社の境界駅は1つであることを仮定している。
   * 仮にJR東海が運賃体系を変えるようなことが起これば大変なことになる。
   * あとSQLのテーブルの組み方自体がlineの会社設定とkiloの会社設定の2つだけで
   * 成り立っているので、lineで設定していない会社同士が接するような状態も
   * 設定できない。ただ、正直どれも起こりそうにないので真面目に実装はしない。
   * @note この実装は2会社にまたがる路線が幹線であることを仮定している。
   * @note 戻り値をboolじゃなくて結果の配列にしたい。
   */
  bool CDatabase::get_company_and_kilo(const line_id_t line,
                                       const station_id_t begin,
                                       const station_id_t end,
                                       std::vector<CKiloValue> & result,
                                       bool & is_main) const
  {
    const char sql[] =
      "SELECT MIN(kilo.kilo), MAX(kilo.kilo), "
      "       line.linecompanyid, kilo.kilocompanyid, line.is_main"
      " FROM kilo NATURAL JOIN line"
      " WHERE lineid=?1 AND kilo BETWEEN"
      "  (SELECT min(kilo) FROM kilo"
      "    WHERE lineid = ?1 AND stationid IN (?2, ?3))"
      "  AND"
      "  (SELECT max(kilo) FROM kilo"
      "    WHERE lineid = ?1 AND stationid IN (?2, ?3))"
      " GROUP BY line.linecompanyid, kilo.kilocompanyid, line.is_main";
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, line);
    stmt.bind(2, begin);
    stmt.bind(3, end);
    company_id_t comp_main=-1, comp_sub=-1;
    // 0 means begin, 1 means end
    int main[2]={0};
    int sub[2]={0};
    int kilo_max=0, kilo_min=INT_MAX;
    int rc;
    while(true)
    {
      rc = stmt.step();
      if(rc != SQLITE_ROW) { break; }
      const int
        kilo_temp_max = stmt.column(1),
        kilo_temp_min = stmt.column(0);
      is_main = static_cast<int>(stmt.column(4));
      kilo_max = std::max(kilo_max, kilo_temp_max);
      kilo_min = std::min(kilo_min, kilo_temp_min);
      // kilocompanyid exists
      if(!stmt.column(3).is_null())
      {
        sub[0] = kilo_temp_min;
        sub[1] = kilo_temp_max;
        comp_sub = stmt.column(3);
      }else{
        main[0] = kilo_temp_min;
        main[1] = kilo_temp_max;
        comp_main = stmt.column(2);
      }
    }
    // DB error check: throwing exception might be better.
    if(rc != SQLITE_DONE)
    {
      std::cerr << db->errmsg() << std::endl;
      return false;
    }
    const int kilo_all = kilo_max - kilo_min;
    // Cannot get results. maybe line, begin, end are bad.
    if(kilo_all == 0) { return false; }
    // if kilo_main & kilo_sub exists, main[] is not correct.
    if(comp_main != -1 && comp_sub != -1)
    {
      // main is near than sub.
      if(main[0] < sub[0]) { main[1] = sub[0]; }
      // sub is near than main. this is not probable in current JR.
      else { main[0] = sub[1]; }
    }
    const int
      kilo_main = main[1] - main[0],
      kilo_sub  =  sub[1] -  sub[0];
    if(kilo_main != 0) { result.push_back(CKiloValue({comp_main, main[0], main[1]})); }
    if(kilo_sub  != 0) { result.push_back(CKiloValue({comp_sub ,  sub[0] , sub[1]})); }
    return true;
   }

  bool CDatabase::is_belong_to_line(line_id_t line, station_id_t station) const
  {
    const std::string sql =
      "SELECT * FROM kilo WHERE lineid = ? AND stationid = ?";
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql);
    stmt.bind(1, line);
    stmt.bind(2, station);
    int rc = stmt.step();
    if (rc == SQLITE_ROW) return true;
    if (rc == SQLITE_DONE) return false;
    std::cerr << db->errmsg() << std::endl;
    return false;
  }

  bool CDatabase::is_contains(const CSegment & range,
                              const station_id_t station) const
  {
    const char sql[] =
      "SELECT kilo FROM kilo WHERE lineid = ?1 AND stationid = ?2"
      " AND kilo BETWEEN"
      "  (SELECT min(kilo) FROM kilo"
      "    WHERE lineid = ?1 AND stationid IN (?3, ?4))"
      "  AND"
      "  (SELECT max(kilo) FROM kilo"
      "    WHERE lineid = ?1 AND stationid IN (?3, ?4))";
    sqlite3_wrapper::SQLiteStmt stmt(*db, sql);
    stmt.bind(1, range.line);
    stmt.bind(2, station);
    stmt.bind(3, range.begin);
    stmt.bind(4, range.end);
    int rc = stmt.step();
    if(rc == SQLITE_ROW) return true;
    if(rc == SQLITE_DONE) return false;
    std::cerr << db->errmsg() << std::endl;
    return false;
  }
}

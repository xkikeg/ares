/* -*-coding: utf-8-*- */
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>

#include "util.hpp"
#include "sqlite3_wrapper.h"
#include "aresutil.h"
#include "cdatabase.h"
#include "csegment.h"
#include "ckilo.h"
#include "cstation.h"

namespace ares
{
  using sqlite3_wrapper::SQLiteStmt;
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

  std::pair<DENSHA_SPECIAL_TYPE, DENSHA_SPECIAL_TYPE>
  CDatabase::get_denshaid(const line_id_t line,
                          const std::pair<int, int> range) const
  {
    const char sql[] =
      "SELECT station.denshaid, station.denshacircleid"
      " FROM station NATURAL JOIN kilo NATURAL JOIN line"
      " WHERE lineid=? AND kilo BETWEEN ? AND ?"
      " GROUP BY station.denshaid, station.denshacircleid";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, line);
    stmt.bind(2, range.first);
    stmt.bind(3, range.second);
    DENSHA_SPECIAL_TYPE denshaid = DENSHA_SPECIAL_NONE; //!< 電車特定区間ID
    DENSHA_SPECIAL_TYPE circleid = DENSHA_SPECIAL_NONE; //!< 山手/環状特例ID
    bool iscircle = true; //!< 山手/環状特例かのフラグ
    for(SQLiteStmt::iterator itr=stmt.execute(); itr; ++itr)
    {
      // 電車特定区間ではない
      if(itr[0].is_null())
      {
        return std::make_pair(DENSHA_SPECIAL_NONE, DENSHA_SPECIAL_NONE);
      }
      // 電車特定区間である
      else { denshaid = DENSHA_SPECIAL_TYPE(static_cast<int>(itr[0])); }
      // 山手線内・大阪環状線内ではない
      if(itr[1].is_null()) { iscircle = false; }
      // 山手線内・大阪環状線内である
      else if(iscircle)
      {
        circleid = DENSHA_SPECIAL_TYPE(static_cast<int>(itr[1]));
      }
    }
    return std::make_pair(denshaid, circleid);
  }

  CDatabase::CDatabase(const char * dbname, bool memcache)
    : db(new SQLite(dbname, SQLITE_OPEN_READONLY))
  {
    if(memcache)
    {
      std::unique_ptr<SQLite> memdb(new SQLite(":memory:"));
      {
        sqlite3_wrapper::SQLiteBackup backup(*memdb, "main",
                                             *db, "main");
        backup.step(-1);
      }
      db = std::move(memdb);
    }
  }

  std::string CDatabase::get_line_name(line_id_t line) const
  {
    const char sql[] = "SELECT linename FROM line WHERE lineid = ?";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, line);
    SQLiteStmt::iterator result=stmt.execute();
    if(!result)
    {
      std::stringstream ss;
      ss << "line id not found: " << line;
      throw std::out_of_range(ss.str());
    }
    return static_cast<const char *>(result[0]);
  }

  std::string CDatabase::get_station_name(station_id_t station) const
  {
    const char sql[] = "SELECT stationname FROM station WHERE stationid = ?";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, station);
    SQLiteStmt::iterator result=stmt.execute();
    if(!result)
    {
      std::stringstream ss;
      ss << "station id not found: " << station;
      throw std::out_of_range(ss.str());
    }
    const char * name = result[0];
    return std::string(name);
  }

  std::string CDatabase::get_station_yomi(station_id_t station) const
  {
    const char sql[] = "SELECT stationyomi FROM station WHERE stationid = ?";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, station);
    SQLiteStmt::iterator result=stmt.execute();
    if(!result)
    {
      std::stringstream ss;
      ss << "station id not found: " << station;
      throw std::out_of_range(ss.str());
    }
    const char * yomi = result[0];
    return std::string(yomi);
  }

  std::string CDatabase::get_station_denryaku(station_id_t station) const
  {
    const char sql[] = "SELECT stationdenryaku FROM station WHERE stationid = ?";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, station);
    SQLiteStmt::iterator result=stmt.execute();
    if(!result)
    {
      std::stringstream ss;
      ss << "station id not found: " << station;
      throw std::out_of_range(ss.str());
    }
    const char * denryaku = result[0];
    return std::string(denryaku);
  }

  void CDatabase::get_all_lines_name(std::vector<std::pair<
                                     line_id_t, std::string> > & result) const
  {
    const char sql[] = "SELECT lineid, linename FROM line ORDER BY lineyomi";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.fill_column(result, 0, 1);
  }

  void CDatabase::get_stations_of_line(line_id_t line,
                                       std::vector<CStation> &result) const
  {
    const char sql[] =
      "SELECT station.stationid, station.stationname, station.stationyomi,"
      "       station.stationdenryaku, kilo.kilo, line.is_main"
      " FROM station NATURAL JOIN kilo NATURAL JOIN line"
      " WHERE line.lineid = ?"
      " ORDER BY kilo.kilo";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, line);
    for(SQLiteStmt::iterator itr=stmt.execute(); itr; ++itr)
    {
      result.push_back(CStation(itr[0],
                                std::string(itr[1]),
                                std::string(itr[2]),
                                std::string(itr[3].is_null() ? "" : itr[3]),
                                itr[4],
                                static_cast<int>(itr[5]) ? itr[4]
                                : CKilo::real2fake(itr[4])));
    }
  }

  void CDatabase::get_lines_of_station(station_id_t station,
                                       line_vector & result) const
  {
    const char sql[] =
      "SELECT lineid"
      " FROM jointkilo"
      " WHERE stationid = ?";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, station);
    for(SQLiteStmt::iterator itr=stmt.execute(); itr; ++itr)
    {
      result.push_back(itr[0]);
    }
  }

  void CDatabase::find_lineid(const char * name,
                              const find_mode mode,
                              line_vector & list) const
  {
    this->find_lineid_with_name(name, mode, list);
    this->find_lineid_with_yomi(name, mode, list);
    this->find_lineid_with_alias(name, mode, list);
  }

  void CDatabase::find_lineid_with_name(const char * name,
                                        const find_mode mode,
                                        line_vector & list) const
  {
    std::string name_(name);
    name_ = add_percent(std::move(name_), mode);
    const char sql[] = "SELECT lineid FROM line WHERE linename LIKE ?;";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, name_);
    stmt.fill_column(list, 0);
  }

  void CDatabase::find_lineid_with_yomi(const char * name,
                                        const find_mode mode,
                                        line_vector & list) const
  {
    std::string name_(name);
    name_ = add_percent(std::move(name_), mode);
    const char sql[] = "SELECT lineid FROM line WHERE lineyomi LIKE ?;";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, name_);
    stmt.fill_column(list, 0);
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
      throw DoesNotExist(name);
    }
    if(v.size() > 1)
    {
      throw MultipleObjectReturned(name, v.size());
    }
    return v[0];
  }

  void CDatabase::find_stationid(const char * name,
                                 const find_mode mode,
                                 station_vector & list) const
  {
    this->find_stationid_with_name(name, mode, list);
    this->find_stationid_with_yomi(name, mode, list);
    this->find_stationid_with_denryaku(name, mode, list);
  }

  void CDatabase::find_stationid_with_name(const char * name,
                                           const find_mode mode,
                                           station_vector & list) const
  {
    const char sql[] = "SELECT stationid FROM station WHERE stationname LIKE ? OR stationname LIKE ?";
    std::string name_norm(name);
    std::string name_paren("（%）");
    name_norm = add_percent(std::move(name_norm), mode);
    name_paren += name_norm;
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, name_norm);
    stmt.bind(2, name_paren);
    stmt.fill_column(list, 0);
  }

  void CDatabase::find_stationid_with_yomi(const char * name,
                                           const find_mode mode,
                                           station_vector & list) const
  {
    // SELECT id FROM station WHERE yomi LIKE 'name%';
    std::string name_(name);
    name_ = add_percent(std::move(name_), mode);
    const char sql[] = "SELECT stationid FROM station WHERE stationyomi LIKE ?;";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, name_);
    stmt.fill_column(list, 0);
  }

  void CDatabase::find_stationid_with_denryaku(const char * name,
                                               const find_mode mode,
                                               station_vector & list) const
  {
    // SELECT id FROM station WHERE denryaku LIKE 'name%' OR denryaku LIKE '__name%';
    std::string name_(name);
    const size_t query_length = ares::u8strlen(name_);
    name_ = add_percent(std::move(name_), mode);
    const char sql[] = "SELECT stationid FROM station WHERE stationdenryaku LIKE ?;";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    if(query_length <= 2)
    {
      std::string name_onlystation("__");
      name_onlystation += name_;
      stmt.bind(1, name_onlystation);
      stmt.fill_column(list, 0);
    }
    else
    {
      stmt.bind(1, name_);
      stmt.fill_column(list, 0);
    }
  }

  station_id_t CDatabase::get_stationid(const char * name,
                                        const find_mode mode) const
  {
    station_vector v;
    find_stationid(name, mode, v);
    if(v.empty())
    {
      throw DoesNotExist(name);
    }
    if(v.size() > 1)
    {
      throw MultipleObjectReturned(name, v.size());
    }
    return v[0];
  }

  void CDatabase::get_connect_line(line_id_t line,
                                   connect_vector & list) const
  {
    const char sql[] =
      "SELECT lineid, stationid FROM jointkilo"
      " WHERE stationid IN ("
      "  SELECT K2.stationid FROM kilo AS K2 WHERE lineid = ?1"
      " ) AND lineid != ?1"
      " ORDER BY stationid, linename";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, line);
    stmt.fill_column(list, 0, 1);
  }

  void CDatabase::get_belong_line(station_id_t station,
                                  line_vector & result) const
  {
    const char sql[] =
      "SELECT lineid FROM jointkilo"
      " WHERE stationid = ? ORDER BY linename";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, station);
    stmt.fill_column(result, 0);
  }

  company_id_t CDatabase::get_company_id(const char * name) const
  {
    const char sql[] =
      "SELECT companyid FROM company WHERE companyname LIKE ?";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, name);
    SQLiteStmt::iterator result = stmt.execute();
    if (result) { return result[0]; }
    return -1;
  }

  std::string CDatabase::get_company_name(const company_id_t id) const
  {
    const char sql[] =
      "SELECT companyname FROM company WHERE companyid = ?";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, id);
    SQLiteStmt::iterator result = stmt.execute();
    if (result) { return static_cast<const char *>(result[0]); }
    std::stringstream ss;
    ss << "Invalid company id: " << id;
    throw std::out_of_range(ss.str());
  }

  int CDatabase::get_fare_table(const char * table,
                                company_id_t company,
                                int kilo) const
  {
    const char sql[] =
      "SELECT fare.fare FROM fare WHERE type = ?1 AND companyid = ?2"
      " AND minkilo <= ?3"
      " AND maxkilo >= ?3";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, table);
    stmt.bind(2, company);
    stmt.bind(3, kilo);
    SQLiteStmt::iterator result = stmt.execute();
    if (result) { return result[0]; }
    std::stringstream ss;
    ss << "Invalid fare table: " << table
       << " company: " << $.get_company_name(company)
       << " kilo: " << kilo;
    throw std::invalid_argument(ss.str());
    return -1;
  }

  boost::optional<int> CDatabase::get_fare_country_table(const char * table,
                                                         company_id_t company,
                                                         int realkilo,
                                                         int fakekilo) const
  {
    const char sql[] =
      "SELECT fare FROM fare_country WHERE type = ?1 AND companyid = ?2"
      " AND (realkilo = ?3    AND fakekilo = ?4"
      "      OR"
      "      realkilo is NULL AND fakekilo = ?4"
      "     )";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, table);
    stmt.bind(2, company);
    stmt.bind(3, realkilo);
    stmt.bind(4, fakekilo);
    SQLiteStmt::iterator result = stmt.execute();
    return result ? boost::make_optional<int>(result[0]) : boost::none;
  }

  int CDatabase::get_kilo(const line_id_t line,
                          const station_id_t station) const
  {
    const char sql[] =
      "SELECT kilo FROM kilo WHERE lineid=? AND stationid=?";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, line);
    stmt.bind(2, station);
    SQLiteStmt::iterator result = stmt.execute();
    if (result) { return result[0]; }
    return -1;
  }

  boost::optional<
    std::pair<bool,int> > CDatabase::get_special_fare(line_id_t line,
                                                      station_id_t begin,
                                                      station_id_t end) const
  {
    const char sql[] =
      "SELECT is_add, fare FROM fare_special"
      " WHERE lineid=?1 "
      "  AND (beginstation=?2 AND endstation=?3 OR"
      "       beginstation=?3 AND endstation=?2)";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, line);
    stmt.bind(2, begin);
    stmt.bind(3, end);
    SQLiteStmt::iterator result = stmt.execute();
    if(!result) return boost::none;
    return std::pair<bool, int>(static_cast<int>(result[0]), result[1]);
  }

  std::pair<int, int> CDatabase::get_range(const line_id_t line,
                                           const station_id_t begin,
                                           const station_id_t end) const
  {
    const char sql[] =
      "SELECT min(kilo), max(kilo) FROM kilo"
      " WHERE lineid = ? AND stationid IN (?, ?)";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, line);
    stmt.bind(2, begin);
    stmt.bind(3, end);
    SQLiteStmt::iterator result = stmt.execute();
    if (result) { return std::make_pair(result[0], result[1]); }
    else
    {
      std::stringstream ss;
      ss << "Invalid line & station in CDatabase::get_range: "
         << "line: " << line
         << "begin: " << begin
         << "end: " << end;
      throw std::invalid_argument(ss.str());
    }
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
                                       bool & is_main,
                                       DENSHA_SPECIAL_TYPE & denshaid,
                                       DENSHA_SPECIAL_TYPE & circleid) const
  {
    std::pair<int, int> range = $.get_range(line, begin, end);
    {
      const std::pair<DENSHA_SPECIAL_TYPE, DENSHA_SPECIAL_TYPE>
        densha = $.get_denshaid(line, range);
      denshaid = densha.first;
      circleid = densha.second;
    }
    const char sql[] =
      "SELECT MIN(kilo.kilo), MAX(kilo.kilo), "
      "       line.linecompanyid, kilo.kilocompanyid, line.is_main"
      " FROM kilo NATURAL JOIN line"
      " WHERE lineid=? AND kilo BETWEEN ? AND ?"
      " GROUP BY line.linecompanyid, kilo.kilocompanyid, line.is_main";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, line);
    stmt.bind(2, range.first);
    stmt.bind(3, range.second);
    company_id_t comp_main=-1, comp_sub=-1;
    // 0 means begin, 1 means end
    int main[2]={0};
    int sub[2]={0};
    int kilo_max=0, kilo_min=INT_MAX;
    for(SQLiteStmt::iterator itr=stmt.execute(), end; itr != end; ++itr)
    {
      const int
        kilo_temp_max = itr[1],
        kilo_temp_min = itr[0];
      is_main = static_cast<int>(itr[4]);
      kilo_max = std::max(kilo_max, kilo_temp_max);
      kilo_min = std::min(kilo_min, kilo_temp_min);
      // kilocompanyid exists
      if(!itr[3].is_null())
      {
        sub[0] = kilo_temp_min;
        sub[1] = kilo_temp_max;
        comp_sub = itr[3];
      }else{
        main[0] = kilo_temp_min;
        main[1] = kilo_temp_max;
        comp_main = itr[2];
      }
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
    if(kilo_main != 0) { result.push_back({comp_main, main[0], main[1]}); }
    if(kilo_sub  != 0) { result.push_back({comp_sub ,  sub[0] , sub[1]}); }
    return true;
  }

  bool CDatabase::is_belong_to_line(line_id_t line, station_id_t station) const
  {
    const char sql[] =
      "SELECT * FROM kilo WHERE lineid = ? AND stationid = ?";
    SQLiteStmt stmt(*db, sql, std::strlen(sql));
    stmt.bind(1, line);
    stmt.bind(2, station);
    SQLiteStmt::iterator result=stmt.execute();
    return static_cast<bool>(result);
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
    SQLiteStmt stmt(*db, sql);
    stmt.bind(1, range.line);
    stmt.bind(2, station);
    stmt.bind(3, range.begin);
    stmt.bind(4, range.end);
    SQLiteStmt::iterator result=stmt.execute();
    return static_cast<bool>(result);
  }
}

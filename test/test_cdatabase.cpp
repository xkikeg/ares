#include <algorithm>
#include <functional>
#include <boost/preprocessor.hpp>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "sqlite3_wrapper.h"
#include "cdatabase.h"
#include "csegment.h"

#define DB_NAME "test.sqlite"

using namespace std::placeholders;
class CDatabaseTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(CDatabaseTest);
  CPPUNIT_TEST(testGetStationNameKanji);
  CPPUNIT_TEST(testGetStationNameYomi);
  CPPUNIT_TEST(testGetStationNameDenryaku);
  CPPUNIT_TEST(testLineConnection);
  CPPUNIT_TEST(testBelongToLine);
  CPPUNIT_TEST(testContains);
  CPPUNIT_TEST_SUITE_END();

  typedef std::pair<std::string, std::string> u8pair_t;
  typedef std::vector<std::string> u8vec_t;
  typedef std::vector<u8pair_t> u8pvec_t;

  std::shared_ptr<ares::CDatabase> db;

public:
  CDatabaseTest() : CPPUNIT_NS::TestFixture(),
                    db(new ares::CDatabase(DB_NAME)) {}

protected:

  void diffStringVector(u8vec_t expected, u8vec_t actual) {
    CPPUNIT_ASSERT_EQUAL(expected.size(), actual.size());
    std::sort(expected.begin(), expected.end());
    std::sort(actual.begin()  , actual.end()  );
    auto i=expected.begin(), j=actual.begin();
    for(; i != expected.end() && j != actual.end(); ++i, ++j)
    {
      CPPUNIT_ASSERT_EQUAL(*i, *j);
    }
  }

  template<class Result, class Getter>
  void diffGetterVector(u8vec_t reference,
                        const Result & idresult,
                        Getter F) {
    u8vec_t result(idresult.size());
    std::transform(idresult.begin(), idresult.end(), result.begin(),
                   std::bind(F, std::ref(*db), _1));
    diffStringVector(std::move(reference), std::move(result));
  }

  void diffStationNameVector(u8vec_t reference,
                             const ares::station_vector & idresult) {
    diffGetterVector(std::move(reference), idresult,
                     &ares::CDatabase::get_station_name);
  }

  void diffLineNameVector(u8vec_t reference,
                          const ares::line_vector & idresult) {
    diffGetterVector(std::move(reference), idresult,
                     &ares::CDatabase::get_line_name);
  }

#define CHECK_GET_STATION_NAME(col)                                     \
  void BOOST_PP_CAT(checkGetStationName_,col)(u8vec_t reference,        \
                                              ares::find_mode mode,     \
                                              const char * query) {     \
    ares::station_vector sidresult;                                     \
    db->BOOST_PP_CAT(find_stationid_with_,col)(query, mode, sidresult); \
    diffStationNameVector(std::move(reference), sidresult);             \
  }

  CHECK_GET_STATION_NAME(name)
  CHECK_GET_STATION_NAME(yomi)
  CHECK_GET_STATION_NAME(denryaku)

  void testGetStationNameKanji() {
    u8vec_t kagoshima_prefix = {
      "鹿児島",
      "鹿児島中央",
    };
    checkGetStationName_name(std::move(kagoshima_prefix),
                               ares::FIND_PREFIX, "鹿児島");
  }

  void testGetStationNameYomi() {
    u8vec_t hassamu_prefix = {
      "発寒",
      "発寒中央",
    };
    checkGetStationName_yomi(std::move(hassamu_prefix),
                               ares::FIND_PREFIX, "はっさむ");
  }

  void testGetStationNameDenryaku() {
    u8vec_t mifu_exact = {
      "南下徳富",
      "南福島",
      "上二田",
      "南古谷",
      "南船橋",
      "身延",
      "南福岡",
      "南由布",
    };
    checkGetStationName_denryaku(std::move(mifu_exact),
                                 ares::FIND_EXACT, "ミフ");
  }

  void checkLineConnection(const char * linename,
                           u8pvec_t expected) {
    ares::line_id_t l;
    ares::connect_vector result;
    CPPUNIT_ASSERT_NO_THROW(l = db->get_lineid(linename));
    db->find_connect_line(l, result);
    u8pvec_t actual(result.size());
    std::transform(result.begin(), result.end(), actual.begin(),
                   [&](ares::station_fqdn_t x) -> u8pair_t {
                     return std::make_pair(db->get_line_name(x.first),
                                           db->get_station_name(x.second));
                   });
    CPPUNIT_ASSERT_EQUAL(expected.size(), actual.size());
    std::sort(expected.begin(), expected.end());
    std::sort(  actual.begin(),   actual.end());
    for(auto i=expected.begin(), j=actual.begin();
        i != expected.end() && j != actual.end(); ++i, ++j)
    {
      CPPUNIT_ASSERT_EQUAL(i->first, j->first);
      CPPUNIT_ASSERT_EQUAL(i->second, j->second);
    }
  }

  void testLineConnection() {
    u8pvec_t kagoshima1 = {
      std::make_pair("山陽"  ,"門司"  ),
      std::make_pair("博多南","博多"  ),
      std::make_pair("筑豊"  ,"折尾"  ),
      std::make_pair("筑豊"  ,"原田"  ),
      std::make_pair("香椎"  ,"香椎"  ),
      std::make_pair("篠栗"  ,"吉塚"  ),
      std::make_pair("日豊"  ,"小倉"  ),
      std::make_pair("日豊"  ,"西小倉"),
      std::make_pair("久大"  ,"久留米"),
      std::make_pair("豊肥"  ,"熊本"  ),
      std::make_pair("三角"  ,"宇土"  ),
      std::make_pair("長崎"  ,"鳥栖"  ),
      std::make_pair("肥薩"  ,"八代"  ),
    };
    checkLineConnection("鹿児島1", std::move(kagoshima1));
  }

  void testBelongToLine() {
    CPPUNIT_ASSERT_EQUAL(true, db->is_belong_to_line(db->get_lineid("鹿児島1"),
                                                     db->get_stationid("鳥栖")));
    CPPUNIT_ASSERT_EQUAL(true, db->is_belong_to_line(db->get_lineid("東海道"),
                                                     db->get_stationid("東京")));
    CPPUNIT_ASSERT_EQUAL(false, db->is_belong_to_line(db->get_lineid("山陽"),
                                                      db->get_stationid("東京")));
  }

  void testContains() {
    ares::CSegment seg1 = ares::CSegment(db->get_stationid("東京"),
                                         db->get_lineid("東海道"),
                                         db->get_stationid("大阪"));
    CPPUNIT_ASSERT_EQUAL(true, db->is_contains(seg1, db->get_stationid("東京")));
    CPPUNIT_ASSERT_EQUAL(true, db->is_contains(seg1, db->get_stationid("京都")));
    CPPUNIT_ASSERT_EQUAL(true, db->is_contains(seg1, db->get_stationid("大阪")));
    CPPUNIT_ASSERT_EQUAL(false, db->is_contains(seg1, db->get_stationid("神戸")));
    CPPUNIT_ASSERT_EQUAL(false, db->is_contains(seg1, db->get_stationid("仙台")));
    seg1.reverse();
    CPPUNIT_ASSERT_EQUAL(true, db->is_contains(seg1, db->get_stationid("東京")));
    CPPUNIT_ASSERT_EQUAL(true, db->is_contains(seg1, db->get_stationid("京都")));
    CPPUNIT_ASSERT_EQUAL(true, db->is_contains(seg1, db->get_stationid("大阪")));
    CPPUNIT_ASSERT_EQUAL(false, db->is_contains(seg1, db->get_stationid("神戸")));
    CPPUNIT_ASSERT_EQUAL(false, db->is_contains(seg1, db->get_stationid("仙台")));
  }

public:

  virtual void setUp() {
  }

  virtual void tearDown() {
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CDatabaseTest);

#include <algorithm>
#include <functional>
#include <boost/preprocessor.hpp>
#include <boost/tr1/functional.hpp>
#include <boost/assign.hpp>
#include "gtest/gtest.h"

#include "sqlite3_wrapper.h"
#include "cdatabase.h"
#include "csegment.h"

#include "test_dbfilename.h"

using namespace std::tr1::placeholders;
using namespace boost::assign;

class CDatabaseTest : public testing::Test
{
public:
  CDatabaseTest() : db(new ares::CDatabase(TEST_DB_FILENAME)) {}

protected:

  std::tr1::shared_ptr<ares::CDatabase> db;
  typedef std::pair<std::string, std::string> u8pair_t;
  typedef std::vector<std::string> u8vec_t;
  typedef std::vector<u8pair_t> u8pvec_t;

  void diffStringVector(u8vec_t expected, u8vec_t actual) {
    EXPECT_EQ(expected.size(), actual.size());
    std::sort(expected.begin(), expected.end());
    std::sort(actual.begin()  , actual.end()  );
    u8vec_t::iterator i=expected.begin(), j=actual.begin();
    for(; i != expected.end() && j != actual.end(); ++i, ++j)
    {
      EXPECT_EQ(*i, *j);
    }
  }

  template<class Result, class Getter>
  void diffGetterVector(u8vec_t reference,
                        const Result & idresult,
                        Getter F) {
    u8vec_t result(idresult.size());
    std::transform(idresult.begin(), idresult.end(), result.begin(),
                   std::tr1::bind(F, std::tr1::ref(*db), _1));
    diffStringVector(reference, result);
  }

  void diffStationNameVector(const u8vec_t & reference,
                             const ares::station_vector & idresult) {
    diffGetterVector(reference, idresult,
                     &ares::CDatabase::get_station_name);
  }

  void diffLineNameVector(const u8vec_t & reference,
                          const ares::line_vector & idresult) {
    diffGetterVector(reference, idresult,
                     &ares::CDatabase::get_line_name);
  }

#define CHECK_GET_STATION_NAME(col)                                     \
  void BOOST_PP_CAT(checkGetStationName_,col)(const u8vec_t &reference, \
                                              ares::find_mode mode,     \
                                              const char * query) {     \
    ares::station_vector sidresult;                                     \
    db->BOOST_PP_CAT(find_stationid_with_,col)(query, mode, sidresult); \
    diffStationNameVector(reference, sidresult);                        \
  }

  CHECK_GET_STATION_NAME(name)
  CHECK_GET_STATION_NAME(yomi)
  CHECK_GET_STATION_NAME(denryaku)

  struct make_line_station_pair
  {
  private:
    std::tr1::shared_ptr<ares::CDatabase> db;
  public:
    make_line_station_pair(std::tr1::shared_ptr<ares::CDatabase> db) : db(db) {}
    u8pair_t operator()(ares::station_fqdn_t x)
    {
      return std::make_pair(db->get_line_name(x.first),
                            db->get_station_name(x.second));
    }
  };

  void checkLineConnection(const char * linename,
                           u8pvec_t expected) {
    ares::line_id_t l;
    ares::connect_vector result;
    EXPECT_NO_THROW(l = db->get_lineid(linename));
    db->find_connect_line(l, result);
    u8pvec_t actual(result.size());
    std::transform(result.begin(), result.end(), actual.begin(),
                   make_line_station_pair(db));
    EXPECT_EQ(expected.size(), actual.size());
    std::sort(expected.begin(), expected.end());
    std::sort(  actual.begin(),   actual.end());
    for(u8pvec_t::iterator i=expected.begin(), j=actual.begin();
        i != expected.end() && j != actual.end(); ++i, ++j)
    {
      EXPECT_EQ(i->first, j->first);
      EXPECT_EQ(i->second, j->second);
    }
  }
};

TEST_F(CDatabaseTest, BadFileName) {
  EXPECT_THROW({
      ares::CDatabase db(".");
    }, ares::IOException);
}

TEST_F(CDatabaseTest, GetStationNameKanji) {
  u8vec_t kagoshima_prefix = list_of
    ("鹿児島")
    ("鹿児島中央");
  checkGetStationName_name(kagoshima_prefix,
                           ares::FIND_PREFIX, "鹿児島");
}

TEST_F(CDatabaseTest, GetStationNameYomi) {
  u8vec_t hassamu_prefix = list_of
    ("発寒")
    ("発寒中央");
  checkGetStationName_yomi(hassamu_prefix,
                           ares::FIND_PREFIX, "はっさむ");
}

TEST_F(CDatabaseTest, GetStationNameDenryaku) {
  u8vec_t mifu_exact = list_of
    ("南下徳富")
    ("南福島")
    ("上二田")
    ("南古谷")
    ("南船橋")
    ("身延")
    ("南福岡")
    ("南由布");
  checkGetStationName_denryaku(mifu_exact,
                               ares::FIND_EXACT, "ミフ");
}

TEST_F(CDatabaseTest, LineConnection) {
  u8pvec_t kagoshima1 = list_of
    (std::make_pair("山陽"  ,"門司"  ))
    (std::make_pair("博多南","博多"  ))
    (std::make_pair("筑豊"  ,"折尾"  ))
    (std::make_pair("筑豊"  ,"原田"  ))
    (std::make_pair("香椎"  ,"香椎"  ))
    (std::make_pair("篠栗"  ,"吉塚"  ))
    (std::make_pair("日豊"  ,"小倉"  ))
    (std::make_pair("日豊"  ,"西小倉"))
    (std::make_pair("久大"  ,"久留米"))
    (std::make_pair("豊肥"  ,"熊本"  ))
    (std::make_pair("三角"  ,"宇土"  ))
    (std::make_pair("長崎"  ,"鳥栖"  ))
    (std::make_pair("肥薩"  ,"八代"  ));
  checkLineConnection("鹿児島1", kagoshima1);
}

TEST_F(CDatabaseTest, BelongToLine) {
  EXPECT_TRUE(db->is_belong_to_line(db->get_lineid("鹿児島1"),
                                        db->get_stationid("鳥栖")));
  EXPECT_TRUE(db->is_belong_to_line(db->get_lineid("東海道"),
                                        db->get_stationid("東京")));
  EXPECT_FALSE(db->is_belong_to_line(db->get_lineid("山陽"),
                                         db->get_stationid("東京")));
}

TEST_F(CDatabaseTest, Contains) {
  ares::CSegment seg1 = ares::CSegment(db->get_stationid("東京"),
                                       db->get_lineid("東海道"),
                                       db->get_stationid("大阪"));
  EXPECT_TRUE(db->is_contains(seg1, db->get_stationid("東京")));
  EXPECT_TRUE(db->is_contains(seg1, db->get_stationid("京都")));
  EXPECT_TRUE(db->is_contains(seg1, db->get_stationid("大阪")));
  EXPECT_FALSE(db->is_contains(seg1, db->get_stationid("神戸")));
  EXPECT_FALSE(db->is_contains(seg1, db->get_stationid("仙台")));
  seg1.reverse();
  EXPECT_TRUE(db->is_contains(seg1, db->get_stationid("東京")));
  EXPECT_TRUE(db->is_contains(seg1, db->get_stationid("京都")));
  EXPECT_TRUE(db->is_contains(seg1, db->get_stationid("大阪")));
  EXPECT_FALSE(db->is_contains(seg1, db->get_stationid("神戸")));
  EXPECT_FALSE(db->is_contains(seg1, db->get_stationid("仙台")));
}

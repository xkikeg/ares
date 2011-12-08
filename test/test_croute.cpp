#include "gtest/gtest.h"

#include "sqlite3_wrapper.h"
#include "croute.h"
#include "cdatabase.h"
#include "test_dbfilename.h"

#ifndef UTF8
#define UTF8(x) (x)
#endif

#define EXPECT_FARE_EQ(expected, route)                   \
  ASSERT_TRUE(route.is_valid());                          \
  EXPECT_EQ(expected, route.calc_fare_inplace()) << route \
  << "\n" << route.get_kilo()

class CRouteTest : public ::testing::Test
{
protected:
  std::tr1::shared_ptr<ares::CDatabase> db;
  ares::CRoute route;

  CRouteTest() : db(new ares::CDatabase(TEST_DB_FILENAME)),
                 route(db) {}
};

class CRouteTokaidoTest : public CRouteTest
{
protected:
  CRouteTokaidoTest() : CRouteTest()
  {
    route = createRouteTokaidoSannyo();
  }

  ares::CRoute createRoute2argsTokaidoSannyo() {
    ares::CRoute route(db, db->get_stationid("東京"));
    route.append_route(UTF8("東海道"), UTF8("神戸"));
    route.append_route(UTF8("山陽"),   UTF8("岡山"));
    return route;
  }

  ares::CRoute createRouteTokaidoSannyo() {
    ares::CRoute route(db);
    route.append_route(UTF8("東海道"), UTF8("東京"), UTF8("神戸"));
    route.append_route(UTF8("山陽"),   UTF8("神戸"), UTF8("岡山"));
    return route;
  }
};

TEST_F(CRouteTokaidoTest, EqualityTwoAndThreeArgs) {
  EXPECT_EQ(createRoute2argsTokaidoSannyo(), route);
}

TEST_F(CRouteTokaidoTest, EqualityIdAndChar) {
  ares::CRoute id_route(db);
  id_route.append_route(db->get_lineid("東海道"),
                        db->get_stationid("東京"), db->get_stationid("神戸"));
  id_route.append_route(db->get_lineid("山陽"),
                        db->get_stationid("神戸"), db->get_stationid("岡山"));
  EXPECT_EQ($.route, id_route);
}

TEST_F(CRouteTokaidoTest, ContainStation) {
  EXPECT_TRUE(route.is_contains(db->get_stationid("東京")));
  EXPECT_TRUE(route.is_contains(db->get_stationid("大阪")));
  EXPECT_TRUE(route.is_contains(db->get_stationid("神戸")));
  EXPECT_TRUE(route.is_contains(db->get_stationid("姫路")));
  EXPECT_TRUE(route.is_contains(db->get_stationid("岡山")));
  EXPECT_FALSE(route.is_contains(db->get_stationid("広島")));
  EXPECT_FALSE(route.is_contains(db->get_stationid("盛岡")));
}

TEST_F(CRouteTokaidoTest, ValidRoute) {
  EXPECT_TRUE(route.is_valid());
}

TEST_F(CRouteTest, ValidSmallPieceRoute) {
  route.append_route(UTF8("東海道"), UTF8("東京"), UTF8("有楽町"));
  route.append_route(UTF8("東海道"), UTF8("有楽町"), UTF8("新橋"));
  route.append_route(UTF8("東海道"), UTF8("新橋"), UTF8("浜松町"));
  route.append_route(UTF8("東海道"), UTF8("浜松町"), UTF8("品川"));
  route.append_route(UTF8("東海道"), UTF8("品川"), UTF8("大井町"));
  route.append_route(UTF8("東海道"), UTF8("大井町"), UTF8("蒲田"));
  route.append_route(UTF8("東海道"), UTF8("蒲田"), UTF8("新子安"));
  EXPECT_TRUE(route.is_valid());
}

TEST_F(CRouteTokaidoTest, ValidDuplicateShinkansen) {
  route.append_route(UTF8("山陽"), UTF8("岡山"), UTF8("三原"));
  route.append_route(UTF8("新幹線"), UTF8("三原"), UTF8("新尾道"));
  EXPECT_TRUE(route.is_valid());
}

TEST_F(CRouteTokaidoTest, InvalidDuplicateRoute) {
  route.append_route(UTF8("山陽"), UTF8("神戸"), UTF8("岡山"));
  EXPECT_FALSE(route.is_valid());
}

TEST_F(CRouteTokaidoTest, InvalidDiscontinuousRoute) {
  route.append_route(UTF8("上越"), UTF8("高崎"), UTF8("土合"));
  EXPECT_FALSE(route.is_valid());
}

TEST_F(CRouteTokaidoTest, InvalidDuplicateShinkansen) {
  route.append_route(UTF8("山陽"),UTF8("岡山"), UTF8("三原"));
  route.append_route(UTF8("新幹線"), UTF8("三原"), UTF8("福山"));
  EXPECT_FALSE(route.is_valid());
}

TEST_F(CRouteTest, CalcHonshuMain) {
  using ares::CRoute;
  EXPECT_EQ(140, CRoute::calc_honshu_main(1));
  EXPECT_EQ(140, CRoute::calc_honshu_main(3));
  EXPECT_EQ(180, CRoute::calc_honshu_main(4));
  EXPECT_EQ(180, CRoute::calc_honshu_main(6));
  EXPECT_EQ(190, CRoute::calc_honshu_main(7));
  EXPECT_EQ(190, CRoute::calc_honshu_main(10));
  EXPECT_EQ(230, CRoute::calc_honshu_main(11));
  EXPECT_EQ(230, CRoute::calc_honshu_main(15));
  EXPECT_EQ(320, CRoute::calc_honshu_main(16));
  EXPECT_EQ(320, CRoute::calc_honshu_main(20));
  EXPECT_EQ(400, CRoute::calc_honshu_main(21));
  EXPECT_EQ(400, CRoute::calc_honshu_main(25));
  EXPECT_EQ(480, CRoute::calc_honshu_main(26));
  EXPECT_EQ(820, CRoute::calc_honshu_main(50));
  EXPECT_EQ(950, CRoute::calc_honshu_main(51));
  EXPECT_EQ(1620, CRoute::calc_honshu_main(100));
  EXPECT_EQ(1890, CRoute::calc_honshu_main(101));
  EXPECT_EQ(4940, CRoute::calc_honshu_main(300));
  EXPECT_EQ(5250, CRoute::calc_honshu_main(301));
  EXPECT_EQ(9030, CRoute::calc_honshu_main(600));
  EXPECT_EQ(9350, CRoute::calc_honshu_main(601));
  EXPECT_EQ(11970, CRoute::calc_honshu_main(999));
}

TEST_F(CRouteTest, FareHonshuMain) {
  // 353.8km
  route.append_route(UTF8("北陸"), UTF8("米原"), UTF8("直江津"));
  EXPECT_FARE_EQ(5780, route);
}

TEST_F(CRouteTest, FareHonshuMainMiddle) {
  // 467.8km
  route.append_route(UTF8("東北"), UTF8("蕨"), UTF8("北上"));
  EXPECT_FARE_EQ(7350, route);
}

TEST_F(CRouteTest, FareHonshuMain2Lines) {
  // 535.0km
  route.append_route(UTF8("東北"), UTF8("蕨"), UTF8("東京"));
  route.append_route(UTF8("東海道"), UTF8("東京"), UTF8("名古屋"));
  route.append_route(UTF8("関西"), UTF8("名古屋"), UTF8("王寺"));
  EXPECT_FARE_EQ(8190, route);
}

TEST_F(CRouteTest, FareHonshuYamanote) {
  // 6.8km
  route.append_route(UTF8("東海道"), UTF8("品川"), UTF8("東京"));
  EXPECT_FARE_EQ(160, route);
  // 13.9km
  route.append_route(UTF8("東北"), UTF8("田端"));
  EXPECT_FARE_EQ(190, route);
}

TEST_F(CRouteTest, FareHonshuOsakaKanjo) {
  // 10.0km
  route.append_route(UTF8("大阪環状"), UTF8("大阪"), UTF8("今宮"));
  route.append_route(UTF8("関西"), UTF8("今宮"), UTF8("新今宮"));
  EXPECT_FARE_EQ(170, route);
  // 14.9km
  route.append_route(UTF8("関西"), UTF8("新今宮"), UTF8("天王寺"));
  route.append_route(UTF8("大阪環状"), UTF8("天王寺"), UTF8("玉造"));
  EXPECT_FARE_EQ(190, route);
}

TEST_F(CRouteTest, FareHonshuTokyoDenshaTokutei) {
  // 7.1km
  route.append_route(UTF8("中央東"), UTF8("御茶ノ水"), UTF8("神田"));
  route.append_route(UTF8("東北"), UTF8("田端"));
  EXPECT_FARE_EQ(160, route);
  // 13.2km
  route.append_route(UTF8("東北"), UTF8("赤羽"));
  EXPECT_FARE_EQ(210, route);
}

TEST_F(CRouteTest, FareHonshuOsakaDenshaTokutei) {
  // 8.9km
  route.append_route(UTF8("関西"), UTF8("JR難波"), UTF8("加美"));
  EXPECT_FARE_EQ(170, route);
  // 10.6km
  route.append_route(UTF8("関西"), UTF8("久宝寺"));
  EXPECT_FARE_EQ(210, route);
}

TEST_F(CRouteTest, FareHonshuShinkansenGantoku) {
  // 88.5km real
  // 92.9km fake
  route.append_route(UTF8("新幹線"), UTF8("広島"), UTF8("徳山"));
  EXPECT_FARE_EQ(1620, route);
}

TEST_F(CRouteTest, FareHonshuShinkansenKokura) {
  // 86.2km real
  route.append_route(UTF8("新幹線"), UTF8("新下関"), UTF8("博多"));
  EXPECT_FARE_EQ(1450, route);
}

TEST_F(CRouteTest, FareHonshuLocalOnly) {
  // 111.6km real
  // 122.8km fake
  route.append_route(UTF8("姫新"), UTF8("佐用"), UTF8("姫路"));
  route.append_route(UTF8("播但"), UTF8("姫路"), UTF8("和田山"));
  EXPECT_FARE_EQ(2210, route);
}

TEST_F(CRouteTest, FareHonshuMainAndLocal) {
  // 110.1km real
  // 119.6km fake
  route.append_route(UTF8("姫新"), UTF8("佐用"), UTF8("姫路"));
  route.append_route(UTF8("山陽"), UTF8("姫路"), UTF8("加古川"));
  route.append_route(UTF8("加古川"), UTF8("加古川"), UTF8("谷川"));
  EXPECT_FARE_EQ(1890, route);
}

TEST_F(CRouteTest, FareHonshuMainAndLocalShort) {
  //  9.7km real
  // 10.0km fake
  route.append_route(UTF8("八高"), UTF8("北八王子"), UTF8("八王子"));
  route.append_route(UTF8("中央東"), UTF8("八王子"), UTF8("日野"));
  EXPECT_FARE_EQ(200, route);
}

TEST_F(CRouteTest, FareHonshuMainAndLocalNotShort) {
  // 11.7km real
  // 12.2km fake
  route.append_route(UTF8("八高"), UTF8("小宮"), UTF8("八王子"));
  route.append_route(UTF8("中央東"), UTF8("八王子"), UTF8("日野"));
  EXPECT_FARE_EQ(230, route);
}

TEST_F(CRouteTest, FareKyushuMain) {
  // 227.1km real
  route.append_route(UTF8("日豊"), UTF8("大分"), UTF8("西小倉"));
  route.append_route(UTF8("鹿児島1"), UTF8("西小倉"), UTF8("鳥栖"));
  EXPECT_FARE_EQ(4200, route);
}

TEST_F(CRouteTest, FareKyushuLocal) {
  // 289.5km real
  // 318.5km real
  route.append_route(UTF8("久大"), UTF8("久留米"), UTF8("大分"));
  route.append_route(UTF8("豊肥"), UTF8("大分"), UTF8("熊本"));
  EXPECT_FARE_EQ(5670, route);
}

TEST_F(CRouteTest, FareKyushuMainAndLocal) {
  // 13.9km real
  // 14.6km real
  route.append_route(UTF8("久大"), UTF8("久留米大学前"), UTF8("久留米"));
  route.append_route(UTF8("鹿児島1"), UTF8("久留米"), UTF8("鳥栖"));
  EXPECT_FARE_EQ(270, route);
}

TEST_F(CRouteTest, FareKyushuAndHonshuMain) {
  // 86.2km real
  // 79.0km kyushu part
  route.append_route(UTF8("山陽"), UTF8("新下関"), UTF8("門司"));
  route.append_route(UTF8("鹿児島1"), UTF8("門司"), UTF8("博多"));
  EXPECT_FARE_EQ(1600, route);
}

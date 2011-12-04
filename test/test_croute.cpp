#include "gtest/gtest.h"

#include "sqlite3_wrapper.h"
#include "croute.h"
#include "cdatabase.h"
#include "test_dbfilename.h"

class CRouteTest : public testing::Test
{
public:
  CRouteTest() : db(new ares::CDatabase(TEST_DB_FILENAME)),
                 route(db) {}

protected:
  std::shared_ptr<ares::CDatabase> db;
  ares::CRoute route;
};

class CRouteTokaidoTest : public CRouteTest
{
public:
  CRouteTokaidoTest() : CRouteTest()
  {
    route = createRouteTokaidoSannyo();
  }

protected:
  ares::CRoute createRoute2argsTokaidoSannyo() {
    ares::CRoute route(db, db->get_stationid("東京"));
    route.append_route(db->get_lineid("東海道"), db->get_stationid("神戸"));
    route.append_route(db->get_lineid("山陽"), db->get_stationid("岡山"));
    return std::move(route);
  }

  ares::CRoute createRouteTokaidoSannyo() {
    ares::CRoute route(db);
    route.append_route(db->get_lineid("東海道"),
                       db->get_stationid("東京"), db->get_stationid("神戸"));
    route.append_route(db->get_lineid("山陽"),
                       db->get_stationid("神戸"), db->get_stationid("岡山"));
    return std::move(route);
  }
};

TEST_F(CRouteTokaidoTest, EqualityTwoAndThreeArgs) {
  EXPECT_EQ(createRoute2argsTokaidoSannyo(), route);
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

TEST_F(CRouteTokaidoTest, ValidDuplicateShinkansen) {
  route.append_route(db->get_lineid("山陽"),
                     db->get_stationid("岡山"), db->get_stationid("三原"));
  route.append_route(db->get_lineid("新幹線"),
                     db->get_stationid("三原"), db->get_stationid("新尾道"));
  EXPECT_TRUE(route.is_valid());
}

TEST_F(CRouteTokaidoTest, InvalidDuplicateRoute) {
  route.append_route(db->get_lineid("山陽"),
                     db->get_stationid("神戸"), db->get_stationid("岡山"));
  EXPECT_FALSE(route.is_valid());
}

TEST_F(CRouteTokaidoTest, InvalidDiscontinuousRoute) {
  route.append_route(db->get_lineid("上越"),
                     db->get_stationid("高崎"), db->get_stationid("土合"));
  EXPECT_FALSE(route.is_valid());
}

TEST_F(CRouteTokaidoTest, InvalidDuplicateShinkansen) {
  route.append_route(db->get_lineid("山陽"),
                     db->get_stationid("岡山"), db->get_stationid("三原"));
  route.append_route(db->get_lineid("新幹線"),
                     db->get_stationid("三原"), db->get_stationid("福山"));
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
  route.append_route(db->get_lineid("北陸"),
                     db->get_stationid("米原"), db->get_stationid("直江津"));
  EXPECT_EQ(5780, route.calc_fare_inplace());
}

TEST_F(CRouteTest, FareHonshuMainMiddle) {
  // 467.8km
  route.append_route(db->get_lineid("東北"),
                     db->get_stationid("蕨"), db->get_stationid("北上"));
  EXPECT_EQ(7350, route.calc_fare_inplace());
}

TEST_F(CRouteTest, FareHonshuMain2Lines) {
  // 535.0km
  route.append_route(db->get_lineid("東北"),
                     db->get_stationid("蕨"), db->get_stationid("東京"));
  route.append_route(db->get_lineid("東海道"),
                     db->get_stationid("東京"), db->get_stationid("名古屋"));
  route.append_route(db->get_lineid("関西"),
                     db->get_stationid("名古屋"), db->get_stationid("王寺"));
  EXPECT_EQ(8190, route.calc_fare_inplace());
}

TEST_F(CRouteTest, FareHonshuShinkansenGantoku) {
  // 88.5km real
  // 92.9km fake
  route.append_route(db->get_lineid("新幹線"),
                     db->get_stationid("広島"), db->get_stationid("徳山"));
  EXPECT_EQ(1620, route.calc_fare_inplace());
}

TEST_F(CRouteTest, FareHonshuShinkansenKokura) {
  // 86.2km real
  route.append_route(db->get_lineid("新幹線"),
                     db->get_stationid("新下関"), db->get_stationid("博多"));
  EXPECT_EQ(1450, route.calc_fare_inplace());
}

TEST_F(CRouteTest, FareHonshuLocalOnly) {
  // 111.6km real
  // 122.8km fake
  route.append_route(db->get_lineid("姫新"),
                     db->get_stationid("佐用"), db->get_stationid("姫路"));
  route.append_route(db->get_lineid("播但"),
                     db->get_stationid("姫路"), db->get_stationid("和田山"));
  EXPECT_EQ(2210, route.calc_fare_inplace());
}

TEST_F(CRouteTest, FareHonshuMainAndLocal) {
  // 110.1km real
  // 119.6km fake
  route.append_route(db->get_lineid("姫新"),
                     db->get_stationid("佐用"), db->get_stationid("姫路"));
  route.append_route(db->get_lineid("山陽"),
                     db->get_stationid("姫路"), db->get_stationid("加古川"));
  route.append_route(db->get_lineid("加古川"),
                     db->get_stationid("加古川"), db->get_stationid("谷川"));
  EXPECT_EQ(1890, route.calc_fare_inplace());
}

TEST_F(CRouteTest, FareKyushuMain) {
  // 227.1km real
  route.append_route(db->get_lineid("日豊"),
                     db->get_stationid("大分"), db->get_stationid("博多"));
  route.append_route(db->get_lineid("鹿児島1"),
                     db->get_stationid("博多"), db->get_stationid("鳥栖"));
  EXPECT_EQ(4200, route.calc_fare_inplace());
}

TEST_F(CRouteTest, FareKyushuLocal) {
  // 289.5km real
  // 318.5km real
  route.append_route(db->get_lineid("日豊"),
                     db->get_stationid("大分"), db->get_stationid("博多"));
  route.append_route(db->get_lineid("鹿児島1"),
                     db->get_stationid("博多"), db->get_stationid("鳥栖"));
  EXPECT_EQ(5670, route.calc_fare_inplace());
}

TEST_F(CRouteTest, FareKyushuMainAndLocal) {
  // 13.9km real
  // 14.6km real
  route.append_route(db->get_lineid("久大"),
                     db->get_stationid("久留米大学前"), db->get_stationid("久留米"));
  route.append_route(db->get_lineid("鹿児島1"),
                     db->get_stationid("久留米"), db->get_stationid("鳥栖"));
  EXPECT_EQ(270, route.calc_fare_inplace());
}

TEST_F(CRouteTest, FareKyushuAndHonshuMain) {
  // 86.2km real
  // 79.0km kyushu part
  route.append_route(db->get_lineid("山陽"),
                     db->get_stationid("新下関"), db->get_stationid("門司"));
  route.append_route(db->get_lineid("鹿児島1"),
                     db->get_stationid("門司"), db->get_stationid("博多"));
  EXPECT_EQ(1600, route.calc_fare_inplace());
}

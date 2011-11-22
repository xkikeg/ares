#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "sqlite3_wrapper.h"
#include "croute.h"
#include "cdatabase.h"

class CRouteTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(CRouteTest);
  CPPUNIT_TEST(testEqualityTwoAndThreeArgs);
  CPPUNIT_TEST(testContainStation);
  CPPUNIT_TEST(testValidRoute);
  CPPUNIT_TEST(testInvalidRoute);
  CPPUNIT_TEST(testCalcHonshuMain);
  CPPUNIT_TEST(testFareHonshuMain);
  CPPUNIT_TEST(testFareHonshuMainMiddle);
  CPPUNIT_TEST(testFareHonshuMain2Lines);
  CPPUNIT_TEST_SUITE_END();

  std::shared_ptr<ares::CDatabase> db;

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

  void testEqualityTwoAndThreeArgs() {
    CPPUNIT_ASSERT_EQUAL(createRoute2argsTokaidoSannyo(),
                         createRouteTokaidoSannyo());
  }

  void testContainStation() {
    ares::CRoute route = createRouteTokaidoSannyo();
    CPPUNIT_ASSERT_EQUAL(true, route.is_contains(db->get_stationid("東京")));
    CPPUNIT_ASSERT_EQUAL(true, route.is_contains(db->get_stationid("大阪")));
    CPPUNIT_ASSERT_EQUAL(true, route.is_contains(db->get_stationid("神戸")));
    CPPUNIT_ASSERT_EQUAL(true, route.is_contains(db->get_stationid("姫路")));
    CPPUNIT_ASSERT_EQUAL(true, route.is_contains(db->get_stationid("岡山")));
    CPPUNIT_ASSERT_EQUAL(false, route.is_contains(db->get_stationid("広島")));
    CPPUNIT_ASSERT_EQUAL(false, route.is_contains(db->get_stationid("盛岡")));
  }

  void testValidRoute() {
    ares::CRoute route = createRouteTokaidoSannyo();
    CPPUNIT_ASSERT_EQUAL(true, route.is_valid());
  }

  void testInvalidRoute() {
    {
      ares::CRoute route = createRouteTokaidoSannyo();
      route.append_route(db->get_lineid("山陽"),
                         db->get_stationid("神戸"), db->get_stationid("岡山"));
      CPPUNIT_ASSERT_EQUAL(false, route.is_valid());
    }
    {
      ares::CRoute route = createRouteTokaidoSannyo();
      route.append_route(db->get_lineid("上越"),
                         db->get_stationid("高崎"), db->get_stationid("土合"));
      CPPUNIT_ASSERT_EQUAL(false, route.is_valid());
    }
  }

  void testCalcHonshuMain() {
    using ares::CRoute;
    CPPUNIT_ASSERT_EQUAL(140, CRoute::calc_honshu_main(1));
    CPPUNIT_ASSERT_EQUAL(140, CRoute::calc_honshu_main(3));
    CPPUNIT_ASSERT_EQUAL(180, CRoute::calc_honshu_main(4));
    CPPUNIT_ASSERT_EQUAL(180, CRoute::calc_honshu_main(6));
    CPPUNIT_ASSERT_EQUAL(190, CRoute::calc_honshu_main(7));
    CPPUNIT_ASSERT_EQUAL(190, CRoute::calc_honshu_main(10));
    CPPUNIT_ASSERT_EQUAL(230, CRoute::calc_honshu_main(11));
    CPPUNIT_ASSERT_EQUAL(230, CRoute::calc_honshu_main(15));
    CPPUNIT_ASSERT_EQUAL(320, CRoute::calc_honshu_main(16));
    CPPUNIT_ASSERT_EQUAL(320, CRoute::calc_honshu_main(20));
    CPPUNIT_ASSERT_EQUAL(400, CRoute::calc_honshu_main(21));
    CPPUNIT_ASSERT_EQUAL(400, CRoute::calc_honshu_main(25));
    CPPUNIT_ASSERT_EQUAL(480, CRoute::calc_honshu_main(26));
    CPPUNIT_ASSERT_EQUAL(820, CRoute::calc_honshu_main(50));
    CPPUNIT_ASSERT_EQUAL(950, CRoute::calc_honshu_main(51));
    CPPUNIT_ASSERT_EQUAL(1620, CRoute::calc_honshu_main(100));
    CPPUNIT_ASSERT_EQUAL(1890, CRoute::calc_honshu_main(101));
    CPPUNIT_ASSERT_EQUAL(4940, CRoute::calc_honshu_main(300));
    CPPUNIT_ASSERT_EQUAL(5250, CRoute::calc_honshu_main(301));
    CPPUNIT_ASSERT_EQUAL(9030, CRoute::calc_honshu_main(600));
    CPPUNIT_ASSERT_EQUAL(9350, CRoute::calc_honshu_main(601));
    CPPUNIT_ASSERT_EQUAL(11970, CRoute::calc_honshu_main(999));
  }

  void testFareHonshuMain() {
    ares::CRoute route(db);
    // 353.8km
    route.append_route(db->get_lineid("北陸"),
                       db->get_stationid("米原"), db->get_stationid("直江津"));
    CPPUNIT_ASSERT_EQUAL(5780, route.calc_fare_inplace());
  }

  void testFareHonshuMainMiddle() {
    ares::CRoute route(db);
    // 467.8km
    route.append_route(db->get_lineid("東北"),
                       db->get_stationid("蕨"), db->get_stationid("北上"));
    CPPUNIT_ASSERT_EQUAL(7350, route.calc_fare_inplace());
  }

  void testFareHonshuMain2Lines() {
    ares::CRoute route(db);
    // 535.0km
    route.append_route(db->get_lineid("東北"),
                       db->get_stationid("蕨"), db->get_stationid("東京"));
    route.append_route(db->get_lineid("東海道"),
                       db->get_stationid("東京"), db->get_stationid("名古屋"));
    route.append_route(db->get_lineid("関西"),
                       db->get_stationid("名古屋"), db->get_stationid("王寺"));
    CPPUNIT_ASSERT_EQUAL(8190, route.calc_fare_inplace());
  }

public:
  virtual void setUp() {
    db.reset(new ares::CDatabase("test.sqlite"));
  }

  virtual void tearDown() {
    db.reset();
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CRouteTest);

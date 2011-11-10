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
  CPPUNIT_TEST(calcHonshuMain);
  CPPUNIT_TEST(calcHonshuMainMiddle);
  CPPUNIT_TEST(calcHonshuMain2Lines);
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

  void calcHonshuMain() {
    ares::CRoute route(db);
    // 353.8km
    route.append_route(db->get_lineid("北陸"),
                       db->get_stationid("米原"), db->get_stationid("直江津"));
    CPPUNIT_ASSERT_EQUAL(5780, route.calc_fare_inplace());
  }

  void calcHonshuMainMiddle() {
    ares::CRoute route(db);
    // 467.8km
    route.append_route(db->get_lineid("東北"),
                       db->get_stationid("蕨"), db->get_stationid("北上"));
    CPPUNIT_ASSERT_EQUAL(7350, route.calc_fare_inplace());
  }

  void calcHonshuMain2Lines() {
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

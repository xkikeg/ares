#include <sstream>
#include <boost/optional.hpp>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "ckilo.h"

using namespace std::placeholders;
class CKiloTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(CKiloTest);
  CPPUNIT_TEST(testDefaultConstructor);
  CPPUNIT_TEST(testSet);
  CPPUNIT_TEST(testAdd);
  CPPUNIT_TEST(testGetKilo);
  CPPUNIT_TEST(testDump);
  CPPUNIT_TEST_SUITE_END();

  boost::optional<ares::CKilo> kilo;

protected:

  void testDefaultConstructor() {
    CPPUNIT_ASSERT_EQUAL(0, kilo->get(ares::COMPANY_HONSHU, true));
    CPPUNIT_ASSERT_EQUAL(0, kilo->get(ares::COMPANY_HONSHU, false));
    CPPUNIT_ASSERT_EQUAL(0, kilo->get(ares::COMPANY_HOKKAIDO, true));
    CPPUNIT_ASSERT_EQUAL(0, kilo->get(ares::COMPANY_HOKKAIDO, false));
    CPPUNIT_ASSERT_EQUAL(0, kilo->get(ares::COMPANY_KYUSHU, true));
    CPPUNIT_ASSERT_EQUAL(0, kilo->get(ares::COMPANY_KYUSHU, false));
    CPPUNIT_ASSERT_EQUAL(0, kilo->get(ares::COMPANY_SHIKOKU, true));
    CPPUNIT_ASSERT_EQUAL(0, kilo->get(ares::COMPANY_SHIKOKU, false));
  }

  void testSet() {
    kilo->set(ares::COMPANY_HONSHU, true, 0, 100);
    kilo->set(ares::COMPANY_SHIKOKU, false, 0, 532);
    CPPUNIT_ASSERT_EQUAL(100, kilo->get(ares::COMPANY_HONSHU, true));
    CPPUNIT_ASSERT_EQUAL(532, kilo->get(ares::COMPANY_SHIKOKU, false));
    CPPUNIT_ASSERT_THROW(kilo->set(ares::MAX_COMPANY_TYPE, true, 0, 100),
                         std::out_of_range);
  }

  void testAdd() {
    kilo->add(ares::COMPANY_HONSHU, true, 0, 105);
    kilo->add(ares::COMPANY_KYUSHU, true, 0, 1234);
    kilo->add(ares::COMPANY_KYUSHU, true, 1111, 2222);
    CPPUNIT_ASSERT_EQUAL(105, kilo->get(ares::COMPANY_HONSHU, true));
    CPPUNIT_ASSERT_EQUAL(2345, kilo->get(ares::COMPANY_KYUSHU, true));
  }

  void testGetKilo() {
    kilo->set(ares::COMPANY_SHIKOKU, true, 0, 0);
    CPPUNIT_ASSERT_EQUAL(0, kilo->get_kilo(ares::COMPANY_SHIKOKU, true));
    kilo->set(ares::COMPANY_SHIKOKU, true, 0, 3);
    CPPUNIT_ASSERT_EQUAL(1, kilo->get_kilo(ares::COMPANY_SHIKOKU, true));
    kilo->set(ares::COMPANY_SHIKOKU, true, 0, 10);
    CPPUNIT_ASSERT_EQUAL(1, kilo->get_kilo(ares::COMPANY_SHIKOKU, true));
    kilo->set(ares::COMPANY_SHIKOKU, true, 0, 135);
    CPPUNIT_ASSERT_EQUAL(14, kilo->get_kilo(ares::COMPANY_SHIKOKU, true));
    kilo->set(ares::COMPANY_SHIKOKU, true, 0, 200);
    CPPUNIT_ASSERT_EQUAL(20, kilo->get_kilo(ares::COMPANY_SHIKOKU, true));
  }

  void testDump() {
    const std::string result1 =
      "本州　: 幹線: 0.0 地方交通線: 0.0 \n"
      "北海道: 幹線: 0.0 地方交通線: 0.0 \n"
      "九州　: 幹線: 0.0 地方交通線: 0.0 \n"
      "四国　: 幹線: 0.0 地方交通線: 0.0 \n",
      result2 =
      "本州　: 幹線: 10.0 地方交通線: 0.0 \n"
      "北海道: 幹線: 0.0 地方交通線: 20.0 \n"
      "九州　: 幹線: 32.1 地方交通線: 55.5 \n"
      "四国　: 幹線: 0.0 地方交通線: 0.0 \n";
    std::stringstream ss;
    ss << *kilo;
    CPPUNIT_ASSERT_EQUAL(result1, ss.str());
    ss.str("");
    kilo->set(ares::COMPANY_HONSHU, true, 0, 100);
    kilo->set(ares::COMPANY_HOKKAIDO, false, 0, 200);
    kilo->set(ares::COMPANY_KYUSHU, true, 0, 321);
    kilo->set(ares::COMPANY_KYUSHU, false, 0, 555);
    ss << *kilo;
    CPPUNIT_ASSERT_EQUAL(result2, ss.str());
  }

public:

  virtual void setUp() {
    kilo = ares::CKilo();
  }

  virtual void tearDown() {
    kilo = boost::none;
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CKiloTest);

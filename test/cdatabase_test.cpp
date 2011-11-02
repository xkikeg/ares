#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "sqlite3_wrapper.h"
#include "cdatabase.h"

class CDatabaseTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(CDatabaseTest);
  CPPUNIT_TEST(test_station);
  CPPUNIT_TEST_SUITE_END();

protected:
  std::shared_ptr<ares::CDatabase> db;

  void test_station(){
  }

public:
  virtual void setUp() {
    db.reset(new ares::CDatabase("../ares.sqlite"));
  }

  virtual void tearDown() {
    db.reset();
  }

protected:
};

CPPUNIT_TEST_SUITE_REGISTRATION(CDatabaseTest);

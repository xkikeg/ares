#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "util.hpp"

class UniqueIntervalTreeTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(UniqueIntervalTreeTest);
  CPPUNIT_TEST(testInsertSuccess);
  CPPUNIT_TEST(testInsertFail);
  CPPUNIT_TEST(testQuery);
  CPPUNIT_TEST_SUITE_END();

  liquid::UniqueIntervalTree<int> tree;

protected:
  void testInsertSuccess() {
    CPPUNIT_ASSERT_EQUAL(true, tree.insert( 0,  5));
    CPPUNIT_ASSERT_EQUAL(true, tree.insert(10, 20));
    CPPUNIT_ASSERT_EQUAL(true, tree.insert( 5, 10));
    CPPUNIT_ASSERT_EQUAL(true, tree.insert(30, 35));
    CPPUNIT_ASSERT_EQUAL(true, tree.insert(20, 25));
    CPPUNIT_ASSERT_EQUAL(true, tree.insert(25, 30));
    CPPUNIT_ASSERT_EQUAL(true, tree.insert(35, 40));
    CPPUNIT_ASSERT_EQUAL(true, tree.insert(-9, -4));
    CPPUNIT_ASSERT_EQUAL(true, tree.insert(-1, -2));
  }

  void testInsertFail() {
    CPPUNIT_ASSERT_EQUAL(true, tree.insert( 3,  9));
    CPPUNIT_ASSERT_EQUAL(true, tree.insert(11, 15));
    CPPUNIT_ASSERT_EQUAL(false, tree.insert( 3,  9));
    CPPUNIT_ASSERT_EQUAL(false, tree.insert( 5, 12));
    CPPUNIT_ASSERT_EQUAL(false, tree.insert( 8, 11));
    CPPUNIT_ASSERT_EQUAL(false, tree.insert( 9, 16));
  }

  void testQuery() {
    CPPUNIT_ASSERT_EQUAL(false, tree.query(0));
    CPPUNIT_ASSERT_EQUAL(true, tree.insert( 0,  5));
    CPPUNIT_ASSERT_EQUAL(true, tree.insert(-5,  0));
    CPPUNIT_ASSERT_EQUAL(true, tree.insert( 6, 10));
    CPPUNIT_ASSERT_EQUAL(true, tree.insert(15, 20));
    CPPUNIT_ASSERT_EQUAL(true, tree.query( 0));
    CPPUNIT_ASSERT_EQUAL(true, tree.query( 3));
    CPPUNIT_ASSERT_EQUAL(true, tree.query( 5));
    CPPUNIT_ASSERT_EQUAL(true, tree.query(-3));
    CPPUNIT_ASSERT_EQUAL(true, tree.query(-5));
    CPPUNIT_ASSERT_EQUAL(true, tree.query( 6));
    CPPUNIT_ASSERT_EQUAL(true, tree.query( 9));
    CPPUNIT_ASSERT_EQUAL(true, tree.query(10));
    CPPUNIT_ASSERT_EQUAL(false, tree.query(13));
    CPPUNIT_ASSERT_EQUAL(false, tree.query(21));
    CPPUNIT_ASSERT_EQUAL(false, tree.query(-6));
  }

public:
  virtual void setUp() {
    tree.clear();
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(UniqueIntervalTreeTest);

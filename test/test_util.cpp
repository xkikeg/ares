#include "gtest/gtest.h"

#include "util.hpp"

class UniqueIntervalTreeTest : public ::testing::Test
{
protected:
  liquid::UniqueIntervalTree<int> tree;
};

TEST_F(UniqueIntervalTreeTest, InsertSuccess) {
  EXPECT_TRUE(tree.insert( 0,  5));
  EXPECT_TRUE(tree.insert(10, 20));
  EXPECT_TRUE(tree.insert( 5, 10));
  EXPECT_TRUE(tree.insert(30, 35));
  EXPECT_TRUE(tree.insert(20, 25));
  EXPECT_TRUE(tree.insert(25, 30));
  EXPECT_TRUE(tree.insert(35, 40));
  EXPECT_TRUE(tree.insert(-9, -4));
  EXPECT_TRUE(tree.insert(-1, -2));
}

TEST_F(UniqueIntervalTreeTest, InsertFail) {
  EXPECT_TRUE(tree.insert( 3,  9));
  EXPECT_TRUE(tree.insert(11, 15));
  EXPECT_FALSE(tree.insert( 3,  9));
  EXPECT_FALSE(tree.insert( 5, 12));
  EXPECT_FALSE(tree.insert( 8, 11));
  EXPECT_FALSE(tree.insert( 9, 16));
}

TEST_F(UniqueIntervalTreeTest, Query) {
  EXPECT_FALSE(tree.query(0));
  EXPECT_TRUE(tree.insert( 0,  5));
  EXPECT_TRUE(tree.insert(-5,  0));
  EXPECT_TRUE(tree.insert( 6, 10));
  EXPECT_TRUE(tree.insert(15, 20));
  EXPECT_TRUE(tree.query( 0));
  EXPECT_TRUE(tree.query( 3));
  EXPECT_TRUE(tree.query( 5));
  EXPECT_TRUE(tree.query(-3));
  EXPECT_TRUE(tree.query(-5));
  EXPECT_TRUE(tree.query( 6));
  EXPECT_TRUE(tree.query( 9));
  EXPECT_TRUE(tree.query(10));
  EXPECT_FALSE(tree.query(13));
  EXPECT_FALSE(tree.query(21));
  EXPECT_FALSE(tree.query(-6));
}

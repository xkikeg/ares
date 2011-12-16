#include <memory>
#include "gtest/gtest.h"

#define LENGTH(x) (sizeof(x)/sizeof((x)[0]))
#include "sqlite3_wrapper.h"

using namespace sqlite3_wrapper;

class SQLiteTest : public ::testing::Test
{
protected:
  std::shared_ptr<SQLite> db;

public:
  SQLiteTest() : db(new SQLite(":memory:")) {}
};

TEST_F(SQLiteTest, SingleSelectTest)
{
  SQLiteStmt::iterator end;

  SQLiteStmt stmt_create(*db,
                         "CREATE TABLE foo ("
                         "   id INTEGER PRIMARY KEY,"
                         "   price INTEGER,"
                         "   item TEXT"
                         ")");
  ASSERT_NO_THROW({
      EXPECT_EQ(end, stmt_create.execute());
    }) << L"creating table failed.";

  int price[] = {250, 180, 90};
  const char * item[] = {"Potato", "Pasta", "Tomato"};
  ASSERT_EQ(LENGTH(price), LENGTH(item));

  SQLiteStmt stmt_insert(*db,
                         "INSERT INTO foo ( price, item ) VALUES (?, ?)");
  for(size_t i=0; i<LENGTH(price); ++i)
  {
    stmt_insert.reset();
    stmt_insert.bind(1, price[i]);
    stmt_insert.bind(2, item[i]);
    ASSERT_NO_THROW({
        EXPECT_EQ(end, stmt_insert.execute());
      }) << L"inserting into table failed.";
  }

  SQLiteStmt stmt_select(*db,
                         "SELECT id, price, item FROM foo;");
  size_t j=0;
  SQLiteStmt::iterator itr = stmt_select.execute();
  EXPECT_NE(itr, end);
  for(;
      itr != end; ++itr)
  {
    ASSERT_NO_THROW({
        const int actual_price = itr[1];
        const char * actual_item = itr[2];
        EXPECT_EQ(price[j], actual_price);
        EXPECT_EQ(item[j], std::string(actual_item));
      }) << L"SQLiteStmt::step failed";
    ++j;
  }
  EXPECT_EQ(LENGTH(price), j);
};

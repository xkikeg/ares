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

class SQLiteBackupTest : public ::testing::Test
{
protected:
  std::shared_ptr<SQLite> db1, db2;

public:
  SQLiteBackupTest()
    : db1(new SQLite(":memory:"))
    , db2(new SQLite(":memory:"))
  {
  }
};

TEST_F(SQLiteBackupTest, CompletionBackup)
{
  const size_t TESTCASE_SIZE = 3;
  const int id[] = {0, 1, 2};
  const char * name[] = {"John", "Tom", "Tamala"};
  db1->exec("CREATE TABLE foo (id INTEGER PRIMARY KEY, name TEXT)");
  db1->exec("INSERT INTO foo VALUES (0, 'John')");
  db1->exec("INSERT INTO foo VALUES (1, 'Tom')");
  db1->exec("INSERT INTO foo VALUES (2, 'Tamala')");
  {
    ASSERT_NO_THROW({
        SQLiteBackup backup(*db2, "main",
                            *db1, "main");
        backup.step(-1);
      });
  }
  SQLiteStmt stmt(*db2, "SELECT id, name FROM foo ORDER BY id;");
  size_t j=0;
  for(SQLiteStmt::iterator itr = stmt.execute();
      itr && j < TESTCASE_SIZE; ++itr)
  {
    ASSERT_NO_THROW({
        EXPECT_EQ(id[j], static_cast<int>(itr[0]));
        EXPECT_STREQ(name[j], static_cast<const char *>(itr[1]))
          << name[j] << " != "
          << static_cast<const char *>(itr[1]);
      }) << L"SQLiteStmt::step failed";
    ++j;
  }
}

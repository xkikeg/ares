#include "gtest/gtest.h"

#include "aresutil.h"

using namespace ares;

TEST(u8strlenTest, ASCII)
{
  EXPECT_EQ(0, u8strlen(u8""));
  EXPECT_EQ(9, u8strlen(u8"ABCDEfghi"));
  EXPECT_EQ(7, u8strlen(u8"'@\n\t+[]"));
  EXPECT_EQ(9, u8strlen(u8"123456789"));
}

TEST(u8strlenTest, JapaneseKana)
{
  EXPECT_EQ(5, u8strlen(u8"あいうえお"));
  EXPECT_EQ(6, u8strlen(u8"コンピュータ"));
  EXPECT_EQ(5, u8strlen(u8"ばびぶべぼ"));
  EXPECT_EQ(6, u8strlen(u8"「」、。（）"));
  EXPECT_EQ(6, u8strlen(u8"IBMのPC"));
}

TEST(u8strlenTest, JapaneseKanji)
{
  EXPECT_EQ(4, u8strlen(u8"春夏秋冬"));
  EXPECT_EQ(4, u8strlen(u8"富士吉田"));
  EXPECT_EQ(3, u8strlen(u8"体=體"));
  EXPECT_EQ(22, u8strlen(u8"日本の文字はひらがな・カタカナ・漢字から成る"));
}

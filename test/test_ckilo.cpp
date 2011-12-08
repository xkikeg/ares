#include <sstream>
#include <boost/optional.hpp>
#include "gtest/gtest.h"

#include "ckilo.h"

class CKiloTest : public ::testing::Test
{
protected:
  ares::CKilo kilo;
};

TEST_F(CKiloTest, DefaultConstructor) {
  EXPECT_EQ(0, kilo.get_rawhecto(ares::COMPANY_HONSHU, true));
  EXPECT_EQ(0, kilo.get_rawhecto(ares::COMPANY_HONSHU, false));
  EXPECT_EQ(0, kilo.get_rawhecto(ares::COMPANY_HOKKAIDO, true));
  EXPECT_EQ(0, kilo.get_rawhecto(ares::COMPANY_HOKKAIDO, false));
  EXPECT_EQ(0, kilo.get_rawhecto(ares::COMPANY_KYUSHU, true));
  EXPECT_EQ(0, kilo.get_rawhecto(ares::COMPANY_KYUSHU, false));
  EXPECT_EQ(0, kilo.get_rawhecto(ares::COMPANY_SHIKOKU, true));
  EXPECT_EQ(0, kilo.get_rawhecto(ares::COMPANY_SHIKOKU, false));
}

TEST_F(CKiloTest, Set) {
  kilo.set(ares::COMPANY_HONSHU, true, 0, 100);
  kilo.set(ares::COMPANY_SHIKOKU, false, 0, 532);
  EXPECT_EQ(100, kilo.get_rawhecto(ares::COMPANY_HONSHU, true));
  EXPECT_EQ(532, kilo.get_rawhecto(ares::COMPANY_SHIKOKU, false));
  EXPECT_THROW(kilo.set(ares::MAX_COMPANY_TYPE, true, 0, 100),
               std::out_of_range);
}

TEST_F(CKiloTest, Add) {
  kilo.add(ares::COMPANY_HONSHU, true, 0, 105);
  kilo.add(ares::COMPANY_KYUSHU, true, 0, 1234);
  kilo.add(ares::COMPANY_KYUSHU, true, 1111, 2222);
  EXPECT_EQ(105, kilo.get_rawhecto(ares::COMPANY_HONSHU, true));
  EXPECT_EQ(2345, kilo.get_rawhecto(ares::COMPANY_KYUSHU, true));
  EXPECT_EQ(11, kilo.get(ares::COMPANY_HONSHU, true));
  EXPECT_EQ(235, kilo.get(ares::COMPANY_KYUSHU, true));
}

TEST_F(CKiloTest, GetKilo) {
  kilo.set(ares::COMPANY_SHIKOKU, true, 0, 0);
  EXPECT_EQ(0, kilo.get(ares::COMPANY_SHIKOKU, true));
  kilo.set(ares::COMPANY_SHIKOKU, true, 0, 3);
  EXPECT_EQ(1, kilo.get(ares::COMPANY_SHIKOKU, true));
  kilo.set(ares::COMPANY_SHIKOKU, true, 0, 10);
  EXPECT_EQ(1, kilo.get(ares::COMPANY_SHIKOKU, true));
  kilo.set(ares::COMPANY_SHIKOKU, true, 0, 135);
  EXPECT_EQ(14, kilo.get(ares::COMPANY_SHIKOKU, true));
  kilo.set(ares::COMPANY_SHIKOKU, true, 0, 200);
  EXPECT_EQ(20, kilo.get(ares::COMPANY_SHIKOKU, true));
}

TEST_F(CKiloTest, Dump) {
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
  ss << kilo;
  EXPECT_EQ(result1, ss.str());
  ss.str("");
  kilo.set(ares::COMPANY_HONSHU, true, 0, 100);
  kilo.set(ares::COMPANY_HOKKAIDO, false, 0, 200);
  kilo.set(ares::COMPANY_KYUSHU, true, 0, 321);
  kilo.set(ares::COMPANY_KYUSHU, false, 0, 555);
  ss << kilo;
  EXPECT_EQ(result2, ss.str());
}

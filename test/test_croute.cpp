#include "gtest/gtest.h"

#include "sqlite3_wrapper.h"
#include "croute.h"
#include "cdatabase.h"
#include "test_dbfilename.h"

#ifndef UTF8
#define UTF8(x) (x)
#endif

#define EXPECT_FARE_EQ(expected, route)                   \
  ASSERT_TRUE(route.is_valid());                          \
  EXPECT_EQ(expected, route.calc_fare_inplace()) << route \
  << "\n" << route.accum().kilo

class CRouteTest : public ::testing::Test
{
protected:
  std::shared_ptr<ares::CDatabase> db;
  ares::CRoute route;

  CRouteTest() : db(new ares::CDatabase(TEST_DB_FILENAME)),
                 route(db) {}

  virtual ~CRouteTest();
};

CRouteTest::~CRouteTest() {}

class CRouteTokaidoTest : public CRouteTest
{
protected:
  CRouteTokaidoTest() : CRouteTest()
  {
    route = createRouteTokaidoSannyo();
  }

  ares::CRoute createRoute2argsTokaidoSannyo() {
    ares::CRoute route(db, db->get_stationid("東京"));
    route.append_route(UTF8("東海道"), UTF8("神戸"));
    route.append_route(UTF8("山陽"),   UTF8("岡山"));
    return std::move(route);
  }

  ares::CRoute createRouteTokaidoSannyo() {
    ares::CRoute route(db);
    route.append_route(UTF8("東海道"), UTF8("東京"), UTF8("神戸"));
    route.append_route(UTF8("山陽"),   UTF8("神戸"), UTF8("岡山"));
    return std::move(route);
  }
};

TEST_F(CRouteTokaidoTest, EqualityTwoAndThreeArgs) {
  EXPECT_EQ(createRoute2argsTokaidoSannyo(), route);
}

TEST_F(CRouteTokaidoTest, EqualityIdAndChar) {
  ares::CRoute id_route(db);
  id_route.append_route(db->get_lineid("東海道"),
                        db->get_stationid("東京"), db->get_stationid("神戸"));
  id_route.append_route(db->get_lineid("山陽"),
                        db->get_stationid("神戸"), db->get_stationid("岡山"));
  EXPECT_EQ($.route, id_route);
}

TEST_F(CRouteTokaidoTest, ContainStation) {
  EXPECT_TRUE(route.is_contains(db->get_stationid("東京")));
  EXPECT_TRUE(route.is_contains(db->get_stationid("大阪")));
  EXPECT_TRUE(route.is_contains(db->get_stationid("神戸")));
  EXPECT_TRUE(route.is_contains(db->get_stationid("姫路")));
  EXPECT_TRUE(route.is_contains(db->get_stationid("岡山")));
  EXPECT_FALSE(route.is_contains(db->get_stationid("広島")));
  EXPECT_FALSE(route.is_contains(db->get_stationid("盛岡")));
}

TEST_F(CRouteTokaidoTest, ValidRoute) {
  EXPECT_TRUE(route.is_valid());
}

TEST_F(CRouteTest, ValidSmallPieceRoute) {
  route.append_route(UTF8("東海道"), UTF8("東京"), UTF8("有楽町"));
  route.append_route(UTF8("東海道"), UTF8("有楽町"), UTF8("新橋"));
  route.append_route(UTF8("東海道"), UTF8("新橋"), UTF8("浜松町"));
  route.append_route(UTF8("東海道"), UTF8("浜松町"), UTF8("品川"));
  route.append_route(UTF8("東海道"), UTF8("品川"), UTF8("大井町"));
  route.append_route(UTF8("東海道"), UTF8("大井町"), UTF8("蒲田"));
  route.append_route(UTF8("東海道"), UTF8("蒲田"), UTF8("新子安"));
  EXPECT_TRUE(route.is_valid());
}

TEST_F(CRouteTokaidoTest, ValidDuplicateShinkansen) {
  route.append_route(UTF8("山陽"), UTF8("岡山"), UTF8("三原"));
  route.append_route(UTF8("新幹線"), UTF8("三原"), UTF8("新尾道"));
  EXPECT_TRUE(route.is_valid());
}

TEST_F(CRouteTokaidoTest, InvalidDuplicateRoute) {
  route.append_route(UTF8("山陽"), UTF8("神戸"), UTF8("岡山"));
  EXPECT_FALSE(route.is_valid());
}

TEST_F(CRouteTokaidoTest, InvalidDiscontinuousRoute) {
  route.append_route(UTF8("上越"), UTF8("高崎"), UTF8("土合"));
  EXPECT_FALSE(route.is_valid());
}

TEST_F(CRouteTokaidoTest, InvalidDuplicateShinkansen) {
  route.append_route(UTF8("山陽"),UTF8("岡山"), UTF8("三原"));
  route.append_route(UTF8("新幹線"), UTF8("三原"), UTF8("福山"));
  EXPECT_FALSE(route.is_valid());
}

TEST_F(CRouteTest, ValidCircle) {
  route.append_route(UTF8("東北"), UTF8("神田"), UTF8("田端"));
  route.append_route(UTF8("山手2"), UTF8("新宿"));
  route.append_route(UTF8("中央東"), UTF8("神田"));
  EXPECT_TRUE(route.is_valid());
}

TEST_F(CRouteTest, Valid6Style) {
  route.append_route(UTF8("東北"), UTF8("東京"), UTF8("田端"));
  route.append_route(UTF8("山手2"), UTF8("新宿"));
  route.append_route(UTF8("中央東"), UTF8("神田"));
  EXPECT_TRUE(route.is_valid());
}

TEST_F(CRouteTest, Invalid6Style) {
  route.append_route(UTF8("東北"), UTF8("神田"), UTF8("田端"));
  route.append_route(UTF8("山手2"), UTF8("新宿"));
  route.append_route(UTF8("中央東"), UTF8("神田"));
  EXPECT_TRUE(route.is_valid());
  route.append_route(UTF8("東北"), UTF8("東京"));
  EXPECT_FALSE(route.is_valid());
}

TEST_F(CRouteTest, Invalid8Style) {
  route.append_route(UTF8("総武2"), UTF8("御茶ノ水"), UTF8("錦糸町"));
  route.append_route(UTF8("総武"), UTF8("西船橋"));
  route.append_route(UTF8("武蔵野"), UTF8("南浦和"));
  route.append_route(UTF8("東北"), UTF8("神田"));
  EXPECT_FALSE(route.is_valid());
  route.append_route(UTF8("中央東"), UTF8("御茶ノ水"));
  EXPECT_FALSE(route.is_valid());
}

TEST_F(CRouteTest, CalcHonshuMain) {
  using ares::CRoute;
  EXPECT_EQ(140, CRoute::calc_honshu_main(1));
  EXPECT_EQ(140, CRoute::calc_honshu_main(3));
  EXPECT_EQ(180, CRoute::calc_honshu_main(4));
  EXPECT_EQ(180, CRoute::calc_honshu_main(6));
  EXPECT_EQ(190, CRoute::calc_honshu_main(7));
  EXPECT_EQ(190, CRoute::calc_honshu_main(10));
  EXPECT_EQ(230, CRoute::calc_honshu_main(11));
  EXPECT_EQ(230, CRoute::calc_honshu_main(15));
  EXPECT_EQ(320, CRoute::calc_honshu_main(16));
  EXPECT_EQ(320, CRoute::calc_honshu_main(20));
  EXPECT_EQ(400, CRoute::calc_honshu_main(21));
  EXPECT_EQ(400, CRoute::calc_honshu_main(25));
  EXPECT_EQ(480, CRoute::calc_honshu_main(26));
  EXPECT_EQ(820, CRoute::calc_honshu_main(50));
  EXPECT_EQ(950, CRoute::calc_honshu_main(51));
  EXPECT_EQ(1620, CRoute::calc_honshu_main(100));
  EXPECT_EQ(1890, CRoute::calc_honshu_main(101));
  EXPECT_EQ(4940, CRoute::calc_honshu_main(300));
  EXPECT_EQ(5250, CRoute::calc_honshu_main(301));
  EXPECT_EQ(9030, CRoute::calc_honshu_main(600));
  EXPECT_EQ(9350, CRoute::calc_honshu_main(601));
  EXPECT_EQ(11970, CRoute::calc_honshu_main(999));
}

TEST_F(CRouteTest, FareHonshuMain) {
  // 353.8km
  route.append_route(UTF8("北陸"), UTF8("米原"), UTF8("直江津"));
  EXPECT_FARE_EQ(5780, route);
}

TEST_F(CRouteTest, FareHonshuMainMiddle) {
  // 467.8km
  route.append_route(UTF8("東北"), UTF8("蕨"), UTF8("北上"));
  EXPECT_FARE_EQ(7350, route);
}

TEST_F(CRouteTest, FareHonshuMain2Lines) {
  // 535.0km
  route.append_route(UTF8("東北"), UTF8("蕨"), UTF8("東京"));
  route.append_route(UTF8("東海道"), UTF8("東京"), UTF8("名古屋"));
  route.append_route(UTF8("関西"), UTF8("名古屋"), UTF8("王寺"));
  EXPECT_FARE_EQ(8190, route);
}

TEST_F(CRouteTest, FareHonshuYamanote) {
  // 6.8km
  route.append_route(UTF8("東海道"), UTF8("品川"), UTF8("東京"));
  EXPECT_FARE_EQ(160, route);
  // 13.9km
  route.append_route(UTF8("東北"), UTF8("田端"));
  EXPECT_FARE_EQ(190, route);
}

TEST_F(CRouteTest, FareHonshuOsakaKanjo) {
  // 10.0km
  route.append_route(UTF8("大阪環状"), UTF8("大阪"), UTF8("今宮"));
  route.append_route(UTF8("関西"), UTF8("今宮"), UTF8("新今宮"));
  EXPECT_FARE_EQ(170, route);
  // 14.9km
  route.append_route(UTF8("関西"), UTF8("新今宮"), UTF8("天王寺"));
  route.append_route(UTF8("大阪環状"), UTF8("天王寺"), UTF8("玉造"));
  EXPECT_FARE_EQ(190, route);
}

TEST_F(CRouteTest, FareHonshuTokyoDenshaTokutei) {
  // 7.1km
  route.append_route(UTF8("中央東"), UTF8("御茶ノ水"), UTF8("神田"));
  route.append_route(UTF8("東北"), UTF8("田端"));
  EXPECT_FARE_EQ(160, route);
  // 13.2km
  route.append_route(UTF8("東北"), UTF8("赤羽"));
  EXPECT_FARE_EQ(210, route);
}

TEST_F(CRouteTest, FareHonshuOsakaDenshaTokutei) {
  // 8.9km
  route.append_route(UTF8("関西"), UTF8("JR難波"), UTF8("加美"));
  EXPECT_FARE_EQ(170, route);
  // 10.6km
  route.append_route(UTF8("関西"), UTF8("久宝寺"));
  EXPECT_FARE_EQ(210, route);
}

TEST_F(CRouteTest, FareHonshuShinkansenGantoku) {
  // 88.5km real
  // 92.9km fake
  route.append_route(UTF8("新幹線"), UTF8("広島"), UTF8("徳山"));
  EXPECT_FARE_EQ(1620, route);
}

TEST_F(CRouteTest, FareHonshuShinkansenKokura) {
  // 86.2km real
  route.append_route(UTF8("新幹線"), UTF8("新下関"), UTF8("博多"));
  EXPECT_FARE_EQ(1450, route);
}

TEST_F(CRouteTest, FareHonshuLocalOnly) {
  // 111.6km real
  // 122.8km fake
  route.append_route(UTF8("姫新"), UTF8("佐用"), UTF8("姫路"));
  route.append_route(UTF8("播但"), UTF8("姫路"), UTF8("和田山"));
  EXPECT_FARE_EQ(2210, route);
}

TEST_F(CRouteTest, FareHonshuMainAndLocal) {
  // 110.1km real
  // 119.6km fake
  route.append_route(UTF8("姫新"), UTF8("佐用"), UTF8("姫路"));
  route.append_route(UTF8("山陽"), UTF8("姫路"), UTF8("加古川"));
  route.append_route(UTF8("加古川"), UTF8("加古川"), UTF8("谷川"));
  EXPECT_FARE_EQ(1890, route);
}

TEST_F(CRouteTest, FareHonshuMainAndLocalShort) {
  //  9.7km real
  // 10.0km fake
  route.append_route(UTF8("八高"), UTF8("北八王子"), UTF8("八王子"));
  route.append_route(UTF8("中央東"), UTF8("八王子"), UTF8("日野"));
  EXPECT_FARE_EQ(200, route);
}

TEST_F(CRouteTest, FareHonshuMainAndLocalNotShort) {
  // 11.7km real
  // 12.2km fake
  route.append_route(UTF8("八高"), UTF8("小宮"), UTF8("八王子"));
  route.append_route(UTF8("中央東"), UTF8("八王子"), UTF8("日野"));
  EXPECT_FARE_EQ(230, route);
}

TEST_F(CRouteTest, FareKyushuMain) {
  // 227.1km real
  route.append_route(UTF8("日豊"), UTF8("大分"), UTF8("西小倉"));
  route.append_route(UTF8("鹿児島1"), UTF8("西小倉"), UTF8("鳥栖"));
  EXPECT_FARE_EQ(4200, route);
}

TEST_F(CRouteTest, FareKyushuMainShinkansenSakura) {
  // 28.6km
  route.append_route(UTF8("九州新幹線"), UTF8("博多"), UTF8("新鳥栖"));
  EXPECT_FARE_EQ(540, route);
  // 35.7km
  route.append_route(UTF8("九州新幹線"), UTF8("久留米"));
  EXPECT_FARE_EQ(720, route);
  // 118.4km
  route.append_route(UTF8("九州新幹線"), UTF8("熊本"));
  EXPECT_FARE_EQ(2070, route);
  // 151.3km
  route.append_route(UTF8("九州新幹線"), UTF8("新八代"));
  EXPECT_FARE_EQ(2730, route);
  // 288.9km
  route.append_route(UTF8("九州新幹線"), UTF8("鹿児島中央"));
  EXPECT_FARE_EQ(5360, route);
}

TEST_F(CRouteTest, FareKyushuLocal) {
  // 289.5km real
  // 318.5km fake
  route.append_route(UTF8("久大"), UTF8("久留米"), UTF8("大分"));
  route.append_route(UTF8("豊肥"), UTF8("大分"), UTF8("熊本"));
  EXPECT_FARE_EQ(5670, route);
}

TEST_F(CRouteTest, FareKyushuLocalSpecialCase) {
  // 2.8km real
  // 3.1km fake
  route.append_route(UTF8("唐津"), UTF8("山本"), UTF8("本牟田部"));
  EXPECT_FARE_EQ(170, route);
  //  9.6km real
  // 10.6km fake
  route.append_route(UTF8("唐津"), UTF8("岩屋"));
  EXPECT_FARE_EQ(240, route);
  // 27.8km real
  // 30.6km fake
  route.append_route(UTF8("唐津"), UTF8("小城"));
  EXPECT_FARE_EQ(580, route);
  // 32.9km real
  // 36.2km fake
  route.append_route(UTF8("唐津"), UTF8("久保田"));
  EXPECT_FARE_EQ(720, route);
}

TEST_F(CRouteTest, FareKyushuMainAndLocalYufu) {
  // 42.5km real
  // 43.2km fake
  route.append_route(UTF8("鹿児島1"), UTF8("博多"), UTF8("久留米"));
  route.append_route(UTF8("久大"), UTF8("久留米大学前"));
  EXPECT_FARE_EQ(820, route);
  // 83.3km real
  // 88.1km fake
  route.append_route(UTF8("久大"), UTF8("日田"));
  EXPECT_FARE_EQ(1600, route);
  //  95.2km real
  // 101.2km fake
  route.append_route(UTF8("久大"), UTF8("天ヶ瀬"));
  EXPECT_FARE_EQ(2070, route);
  // 108.9km real
  // 116.2km fake
  route.append_route(UTF8("久大"), UTF8("豊後森"));
  EXPECT_FARE_EQ(2070, route);
  // 134.8km real
  // 144.7km fake
  route.append_route(UTF8("久大"), UTF8("由布院"));
  EXPECT_FARE_EQ(2730, route);
  // 177.2km real
  // 191.4km fake
  route.append_route(UTF8("久大"), UTF8("大分"));
  EXPECT_FARE_EQ(3570, route);
}

TEST_F(CRouteTest, FareKyushuMainAndLocalSpecialCase) {
  //  9.7km real
  // 10.2km fake
  route.append_route(UTF8("筑豊"), UTF8("二島"), UTF8("折尾"));
  route.append_route(UTF8("鹿児島1"), UTF8("黒崎"));
  EXPECT_FARE_EQ(240, route);
}

TEST_F(CRouteTest, FareKyushuMainAndLocalNonSpecialCase) {
  // 切り上げ営業キロが11kmになってしまうので救済措置は取られない.
  // 10.3km real
  // 10.7km fake
  route.append_route(UTF8("唐津"), UTF8("鬼塚"), UTF8("山本"));
  route.append_route(UTF8("筑肥"), UTF8("西相知"));
  EXPECT_FARE_EQ(270, route);
}

TEST_F(CRouteTest, FareKyushuAndHonshuMain) {
  // 86.2km real
  // 79.0km kyushu part
  route.append_route(UTF8("山陽"), UTF8("新下関"), UTF8("門司"));
  route.append_route(UTF8("鹿児島1"), UTF8("門司"), UTF8("博多"));
  EXPECT_FARE_EQ(1600, route);
}

TEST_F(CRouteTest, FareHokkaidoMainHakodate) {
  // 423.1km
  route.append_route(UTF8("函館"), UTF8("函館"), UTF8("旭川"));
  EXPECT_FARE_EQ(7140, route);
}

TEST_F(CRouteTest, FareHokkaidoMainHokuto) {
  // 318.7km
  route.append_route(UTF8("函館"), UTF8("函館"), UTF8("長万部"));
  route.append_route(UTF8("室蘭"), UTF8("沼ノ端"));
  route.append_route(UTF8("千歳"), UTF8("（函）白石"));
  route.append_route(UTF8("函館"), UTF8("札幌"));
  EXPECT_FARE_EQ(5560, route);
}

TEST_F(CRouteTest, FareHokkaidoMainOzora) {
  // 348.5km
  route.append_route(UTF8("根室"), UTF8("釧路"), UTF8("新得"));
  route.append_route(UTF8("石勝"), UTF8("南千歳"));
  route.append_route(UTF8("千歳"), UTF8("（函）白石"));
  route.append_route(UTF8("函館"), UTF8("札幌"));
  EXPECT_FARE_EQ(6090, route);
}

TEST_F(CRouteTest, FareHokkaidoLocalSohya) {
  // 396.2km real
  // 422.1km fake
  route.append_route(UTF8("宗谷"), UTF8("稚内"), UTF8("旭川"));
  route.append_route(UTF8("函館"), UTF8("札幌"));
  EXPECT_FARE_EQ(7140, route);
}

TEST_F(CRouteTest, FareHokkaidoMainAndLocal) {
  // 5.9km real
  // 6.3km fake
  route.append_route(UTF8("函館"), UTF8("琴似"), UTF8("桑園"));
  route.append_route(UTF8("札沼"), UTF8("新川"));
  EXPECT_FARE_EQ(200, route) << L"210なら10km以下特例の処理ミスかも";
  //  9.5km real
  // 10.2km fake
  route.append_route(UTF8("札沼"), UTF8("太平"));
  EXPECT_FARE_EQ(220, route) << L"260なら10km以下特例の処理ミスかも";
  // 28.1km real
  // 30.7km fake
  route.append_route(UTF8("札沼"), UTF8("石狩当別"));
  EXPECT_FARE_EQ(620, route);
}

TEST_F(CRouteTest, FareHokkaidoMainAndLocalOkhotsk) {
  // 113.2km real
  // 124.5km fake
  route.append_route(UTF8("石北"), UTF8("網走"), UTF8("遠軽"));
  EXPECT_FARE_EQ(2420, route);
  // 237.7km real
  // 261.5km fake
  route.append_route(UTF8("石北"), UTF8("新旭川"));
  route.append_route(UTF8("宗谷"), UTF8("旭川"));
  EXPECT_FARE_EQ(4930, route);
  // 333.9km real
  // 357.7km fake
  route.append_route(UTF8("函館"), UTF8("岩見沢"));
  EXPECT_FARE_EQ(6090, route);
  // 374.5km real
  // 398.3km fake
  route.append_route(UTF8("函館"), UTF8("札幌"));
  EXPECT_FARE_EQ(6610, route);
}

TEST_F(CRouteTest, FareHokkaidoAndHonshuHakucho)
{
  // 44.4km real
  // 48.8km fake
  route.append_route(UTF8("津軽"), UTF8("青森"), UTF8("中小国"));
  route.append_route(UTF8("海峡"), UTF8("津軽今別"));
  EXPECT_FARE_EQ(850, route);
  // 157.0km real
  // 172.7km fake
  route.append_route(UTF8("海峡"), UTF8("木古内"));
  route.append_route(UTF8("江差"), UTF8("五稜郭"));
  EXPECT_FARE_EQ(3150, route);
  // 160.4km real
  // 176.1km fake
  route.append_route(UTF8("函館"), UTF8("函館"));
  EXPECT_FARE_EQ(3150, route);
}

TEST_F(CRouteTest, FareShikokuMainShimanto)
{
  route.append_route(UTF8("予讃"), UTF8("（讃）高松"), UTF8("坂出"));
  EXPECT_FARE_EQ(440, route);
  route.append_route(UTF8("予讃"), UTF8("宇多津"));
  EXPECT_FARE_EQ(540, route);
  route.append_route(UTF8("予讃"), UTF8("多度津"));
  EXPECT_FARE_EQ(640, route);
  route.append_route(UTF8("土讃"), UTF8("阿波池田"));
  EXPECT_FARE_EQ(1410, route);
  route.append_route(UTF8("土讃"), UTF8("後免"));
  EXPECT_FARE_EQ(2660, route);
  route.append_route(UTF8("土讃"), UTF8("高知"));
  EXPECT_FARE_EQ(2660, route);
  route.append_route(UTF8("土讃"), UTF8("佐川"));
  EXPECT_FARE_EQ(3400, route);
  route.append_route(UTF8("土讃"), UTF8("須崎"));
  EXPECT_FARE_EQ(3710, route);
  route.append_route(UTF8("土讃"), UTF8("窪川"));
  EXPECT_FARE_EQ(4030, route);
  // 社線のテスト
  route.append_route(UTF8("土佐くろしお"), UTF8("中村"));
  EXPECT_FARE_EQ(5090, route);
  route.append_route(UTF8("土佐くろしお"), UTF8("宿毛"));
  EXPECT_FARE_EQ(5580, route);
}

class CRouteHonshibisanTest : public CRouteTest
{
protected:
  CRouteHonshibisanTest() : CRouteTest()
  {
    route.append_route(UTF8("宇野"), UTF8("岡山"), UTF8("茶屋町"));
    route.append_route(UTF8("本四備讃"), UTF8("宇多津"));
  }
};

TEST_F(CRouteHonshibisanTest, FareShikokuAndHonshuUzushio)
{
  // 71.8km
  route.append_route(UTF8("予讃"), UTF8("（讃）高松"));
  EXPECT_FARE_EQ(1470, route);
  // 146.3km
  route.append_route(UTF8("高徳"), UTF8("徳島"));
  EXPECT_FARE_EQ(2760, route);
}

TEST_F(CRouteHonshibisanTest, FareShikokuAndHonshuShiokaze)
{
  // 76.5km
  route.append_route(UTF8("予讃"), UTF8("観音寺"));
  EXPECT_FARE_EQ(1480, route);
  // 123.1km
  route.append_route(UTF8("予讃"), UTF8("新居浜"));
  EXPECT_FARE_EQ(2440, route);
  // 134.3km
  route.append_route(UTF8("予讃"), UTF8("伊予西条"));
  EXPECT_FARE_EQ(2450, route);
  // 164.9km
  route.append_route(UTF8("予讃"), UTF8("今治"));
  EXPECT_FARE_EQ(3180, route);
  // 214.4km
  route.append_route(UTF8("予讃"), UTF8("松山"));
  EXPECT_FARE_EQ(3810, route);
}

TEST_F(CRouteTest, FareShasenHokutosei)
{
  route.append_route(UTF8("東北"), UTF8("上野"), UTF8("盛岡"));
  EXPECT_FARE_EQ(8190, route);
  route.append_route(UTF8("いわて銀河"), UTF8("目時"));
  EXPECT_FARE_EQ(10490, route);
  route.append_route(UTF8("青い森鉄道"), UTF8("青森"));
  EXPECT_FARE_EQ(13520, route);
}

class CRouteOminatoTest : public CRouteTest
{
protected:
  CRouteOminatoTest() : CRouteTest()
  {
    route.append_route(UTF8("大湊"), UTF8("大湊"), UTF8("野辺地"));
  }
};

TEST_F(CRouteOminatoTest, FareShasenShimokitaAomori)
{
  route.append_route(UTF8("青い森鉄道"), UTF8("青森"));
  EXPECT_FARE_EQ(2120, route);
}

TEST_F(CRouteOminatoTest, FareShasenShimokitaHachinohe)
{
  route.append_route(UTF8("青い森鉄道"), UTF8("八戸"));
  EXPECT_FARE_EQ(2410, route);
  route.append_route(UTF8("青い森鉄道"), UTF8("目時"));
  EXPECT_FARE_EQ(2860, route);
}

TEST_F(CRouteTest, FareShasenKTRHashidate)
{
  route.append_route(UTF8("山陰"), UTF8("京都"), UTF8("福知山"));
  EXPECT_FARE_EQ(1450, route);
  route.append_route(UTF8("宮福(KTR)"), UTF8("宮津"));
  EXPECT_FARE_EQ(2130, route);
  route.append_route(UTF8("宮津(KTR)"), UTF8("天橋立"));
  EXPECT_FARE_EQ(2200, route);
  route.append_route(UTF8("宮津(KTR)"), UTF8("豊岡"));
  EXPECT_FARE_EQ(3200, route);
}

#pragma once

#include <stdexcept>
#include "util.hpp"
#include "ares.h"

namespace ares
{
  /**
   * @~
   * 運賃体系上の会社の列挙体.
   */
  enum COMPANY_TYPE {
    COMPANY_HONSHU,
    COMPANY_HOKKAIDO,
    COMPANY_KYUSHU,
    COMPANY_SHIKOKU,
    MAX_COMPANY_TYPE,
  };

  /**
   * @~
   * 幹線と地方交通線の列挙体.
   */
  enum LINE_TYPE {
    LINE_LOCAL,
    LINE_MAIN,
    MAX_LINE_TYPE,
  };

  /**
   * @~
   * 実キロと擬制キロの列挙体.
   */
  enum KILO_TYPE {
    KILO_REAL,
    KILO_FAKE,
    MAX_KILO_TYPE,
  };

  namespace
  {
    inline size_t linetype(bool is_main)
    {
      return (is_main) ? LINE_MAIN : LINE_LOCAL;
    }

    inline size_t kilotype(bool is_real)
    {
      return (is_real) ? KILO_REAL : KILO_FAKE;
    }

    const char * COMPANY_TYPE_LABEL[ares::MAX_COMPANY_TYPE] =
    {
      "本州　",
      "北海道",
      "九州　",
      "四国　",
    };

    const char * LINE_TYPE_LABEL[MAX_COMPANY_TYPE] =
    {
      "地方交通線",
      "幹線",
    };
  }

  /**
   * @~
   * 営業キロの基本単位を表現するクラス.
   * 会社と始点・終点の営業キロで表現される.
   */
  struct CKiloValue {
    company_id_t company;
    int begin, end;
  };

  /**
   * @~
   * 営業キロを抽象化したクラス.
   * 各会社ごとに営業キロの合計を保存する.
   * 営業キロは0.1km単位で管理されているので,
   * 10倍した整数値として保持している.
   */
  class CKilo
  {
  private:

    int kilo[MAX_COMPANY_TYPE][MAX_LINE_TYPE][MAX_KILO_TYPE];

    void check_boundary(size_t i) const {
      if(i >= MAX_COMPANY_TYPE)
      {
        throw std::out_of_range("too large company index");
      }
    }


    void set(size_t i, bool is_main, bool is_real, int kilo) {
      check_boundary(i);
      $.kilo[i][linetype(is_main)][kilotype(is_real)] = kilo;
    }

  public:

    CKilo() : kilo({{{0}}}) {}

    /**
     * @~
     * 営業キロを加算する関数.
     * @param[in] i       会社を指定するindex
     * @param[in] is_main trueなら幹線, falseなら地方交通線
     * @param[in] begin   発駅の営業キロ
     * @param[in] end     着駅の営業キロ
     */
    void add(size_t i, bool is_main, int begin, int end) {
      $.set(i, is_main,  true, $.get(i, is_main,  true) + (end - begin));
      $.set(i, is_main, false, $.get(i, is_main, false) + real2fake(begin, end));
    }

    /**
     * @~
     * 営業キロの10倍を取得する関数.
     * @param[in] i       会社を指定するindex
     * @param[in] is_main trueなら幹線, falseなら地方交通線
     * @param[in] is_real trueなら実キロ, falseなら擬制キロ
     * @return            営業キロの10倍の値
     */
    int get(size_t i, bool is_main, bool is_real=true) const {
      check_boundary(i);
      return $.kilo[i][linetype(is_main)][kilotype(is_real)];
    }

    /**
     * @~
     * 切り上げ済みの営業キロを取得する関数.
     * @param[in] i       会社を指定するindex
     * @param[in] is_main trueなら幹線, falseなら地方交通線
     * @param[in] is_real trueなら実キロ, falseなら擬制キロ
     * @return            切り上げした営業キロ
     */
    int get_kilo(size_t i, bool is_main, bool is_real=true) const {
      const int kilo10 = $.get(i, is_main, is_real);
      return kilo10 / 10 + (kilo10 % 10 ? 1 : 0);
    }

    /**
     * @~
     * 営業キロを設定する関数.
     * @param[in] i       会社を指定するindex
     * @param[in] is_main trueなら幹線, falseなら地方交通線
     * @param[in] begin   発駅の営業キロ
     * @param[in] end     着駅の営業キロ
     */
    void set(size_t i, bool is_main, int begin, int end) {
      $.set(i, is_main, true, end - begin);
      $.set(i, is_main, false, real2fake(begin, end));
    }

    /**
     * @~
     * 指定された会社の営業キロが0であるかを調べる.
     * @param[in] i  会社を指定するindex
     * @retval true  指定された会社の営業キロが0
     * @retval false 指定された会社の営業キロが0でない
     */
    bool is_zero(size_t i) const {
      check_boundary(i);
      return (kilo[i][0][0] == 0 && kilo[i][1][0] == 0);
    }

    // bool is_zero_honshu()   { return is_zero(0); }
    // bool is_zero_hokkaido() { return is_zero(1); }
    // bool is_zero_kyushu()   { return is_zero(2); }
    // bool is_zero_shikoku()  { return is_zero(3); }

    /**
     * @~
     * 指定された会社にだけ営業キロが設定されているかを調べる.
     * @param[in] idx 会社を指定するindex
     * @retval true   指定された会社以外の営業キロが0
     * @retval false  指定された会社以外の営業キロが非負
     */
    bool is_only(size_t idx) const {
      check_boundary(idx);
      for(size_t i=0; i<MAX_COMPANY_TYPE; ++i)
      {
        if(i != idx && !is_zero(i)) { return false; }
      }
      return true;
    }

    /**
     * @~
     * (10倍された)実キロを擬制キロに変換する.
     * @param[in] realkilo 実キロの10倍
     * @return             擬制キロの10倍
     */
    inline static int real2fake(int realkilo)
    {
      return (realkilo*10 + realkilo + 5) / 10;
    }

    /**
     * @~
     * 始点と終点の営業キロ程から擬制キロを求める.
     * @param[in] realbegin 始点の実キロの10倍
     * @param[in] realend   終点の実キロの10倍
     * @return              擬制キロの10倍
     */
    inline static int real2fake(int realbegin, int realend)
    {
      return real2fake(realend) - real2fake(realbegin);
    }

    /**
     * @~
     * ストリームへの出力関数.
     */
    friend std::ostream & operator<<(std::ostream & ost, CKilo & kilo) {
      for(int i=0; i != MAX_COMPANY_TYPE; ++i)
      {
        ost << COMPANY_TYPE_LABEL[i] << ": ";
        for(int j=MAX_LINE_TYPE; j; )
        {
          --j;
          ost << LINE_TYPE_LABEL[j] << ": "
              << kilo.kilo[i][j][KILO_REAL]/10 << "."
              << kilo.kilo[i][j][KILO_REAL]%10 << " ";
        }
        ost << '\n';
      }
      return ost;
    }
  };
}

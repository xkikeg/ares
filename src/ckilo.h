#pragma once

#include <sstream>
#include <ostream>
#include <stdexcept>
#include <boost/optional.hpp>
#include "util.hpp"
#include "ares.h"

namespace ares
{
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

    const char * DENSHA_TYPE_LABEL[] =
    {
      "該当なし",
      "電車特定区間(東京)",
      "電車特定区間(大阪)",
      "山手線内",
      "大阪環状線内",
    };

    template<class T>
    void set_default_if_changed(boost::optional<T> & target,
                                const T newval,
                                const T DEFAULT_VALUE)
    {
      target = (target && *target != newval) ? DEFAULT_VALUE : newval;
    }
  }

  /**
   * @~
   * 営業キロの基本単位を表現するクラス.
   * 会社と始点・終点の営業キロの10倍で表現される.
   */
  struct CKiloValue {
    company_id_t company;
    int begin, end;
  };

  /**
   * @~
   * 営業キロの10倍(ヘクトメートル)を管理するクラス.
   */
  class CHecto
  {
  private:
    int hecto;

  public:
    CHecto() : hecto(0) {}

    explicit CHecto(int hecto) : hecto(hecto) {}

    /**
     * @~
     * 10倍営業キロを切り上げて実際の営業キロにする関数.
     * @param[in] hecto 営業キロの10倍
     * @return          切り上げした営業キロ
     */
    static int hecto2kilo(int hecto) {
      return hecto / 10 + (hecto % 10 ? 1 : 0);
    }

    //! 加算演算子.
    friend class CHecto operator+(const CHecto & a, const CHecto & b)
    {
      return CHecto(a.hecto + b.hecto);
    }

    //! 加算代入演算子.
    class CHecto & operator+=(const CHecto & b)
    {
      $.hecto += b.hecto;
      return $;
    }

    //! 営業キロを取得するメンバ
    int get_kilo() const { return hecto2kilo($.hecto); }

    //! 営業キロの10倍を取得するメンバ
    int get_hecto() const { return $.hecto; }

    //! same as get_kilo() member.
    operator int() const { return $.get_kilo(); }

    friend std::ostream & operator<<(std::ostream & ost,
                                     const CHecto & hecto) {
      ost << hecto.hecto / 10 << "." << hecto.hecto % 10;
      return ost;
    }

    std::string to_str() const
    {
      std::stringstream ss;
      ss << $;
      return ss.str();
    }
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
    boost::optional<DENSHA_SPECIAL_TYPE> denshaid, circleid;

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
      $.set(i, is_main,  true,
            $.get_rawhecto(i, is_main,  true) + (end - begin));
      $.set(i, is_main, false,
            $.get_rawhecto(i, is_main, false) + real2fake(begin, end));
    }

    /**
     * @~
     * 営業キロの10倍の値をintで取得する関数.
     * @param[in] i       会社を指定するindex
     * @param[in] is_main trueなら幹線, falseなら地方交通線
     * @param[in] is_real trueなら実キロ, falseなら擬制キロ
     * @return            営業キロの10倍の値
     */
    int get_rawhecto(size_t i, bool is_main, bool is_real=true) const {
      check_boundary(i);
      return $.kilo[i][linetype(is_main)][kilotype(is_real)];
    }

    /**
     * @~
     * 営業キロの10倍を取得する関数.
     * @param[in] i       会社を指定するindex
     * @param[in] is_main trueなら幹線, falseなら地方交通線
     * @param[in] is_real trueなら実キロ, falseなら擬制キロ
     * @return            営業キロ
     */
    class CHecto get(size_t i, bool is_main, bool is_real=true) const {
      return std::move(CHecto($.get_rawhecto(i, is_main, is_real)));
    }

    /**
     * @~
     * ＪＲ区間すべての営業キロの10倍を取得する関数.
     * @param[in] is_main trueなら幹線, falseなら地方交通線
     * @param[in] is_real trueなら実キロ, falseなら擬制キロ
     * @return            営業キロ
     */
    class CHecto get_all(bool is_main, bool is_real=true) const {
      CHecto hecto;
      for(size_t i=0; i<MAX_COMPANY_TYPE; ++i)
      {
        hecto += $.get(i, is_main, is_real);
      }
      return hecto;
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

    bool is_all_zero() const {
      for(size_t i=0; i<MAX_COMPANY_TYPE; ++i)
      {
        if(!is_zero(i)) { return false; }
      }
      return true;
    }

    /**
     * @~
     * 指定された会社にだけ営業キロが設定されているかを調べる.
     * @note 指定された会社自体の営業キロが0でもtrueが返る.
     * @param[in] idx 会社を指定するindex
     * @retval true   指定された会社以外の営業キロが0.
     * @retval false  指定された会社以外の営業キロが非ゼロ.
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
     * 全会社の中で唯一値のある会社IDを返す.
     * @retval 会社ID その会社IDのみ営業キロが非ゼロ.
     * @retval 無効値 すべての会社のキロがゼロ, もしくは複数会社で非ゼロ.
     */
    boost::optional<COMPANY_TYPE> get_only() const {
      boost::optional<COMPANY_TYPE> ret;
      for(size_t i=0; i < MAX_COMPANY_TYPE; ++i)
      {
        // i-th company exists.
        if(!is_zero(i))
        {
          // already exists non-zero company.
          if(ret) { return boost::none; }
          // first non-zero company.
          else { ret = COMPANY_TYPE(i); }
        }
      }
      return ret;
    }

    void update_denshaid(const DENSHA_SPECIAL_TYPE new_denshaid,
                         const DENSHA_SPECIAL_TYPE new_circleid)
    {
      set_default_if_changed($.denshaid, new_denshaid, DENSHA_SPECIAL_NONE);
      set_default_if_changed($.circleid, new_circleid, DENSHA_SPECIAL_NONE);
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

    DENSHA_SPECIAL_TYPE get_densha_and_circleid() const
    {
      const DENSHA_SPECIAL_TYPE circle = $.get_circleid();
      return (circle != DENSHA_SPECIAL_NONE) ? circle : $.get_denshaid();
    }

    DENSHA_SPECIAL_TYPE get_denshaid() const
    {
      return $.denshaid ? *$.denshaid : DENSHA_SPECIAL_NONE;
    }

    DENSHA_SPECIAL_TYPE get_circleid() const
    {
      return $.circleid ? *$.circleid : DENSHA_SPECIAL_NONE;
    }

    /**
     * @~
     * ストリームへの出力関数.
     */
    friend std::ostream & operator<<(std::ostream & ost, const CKilo & kilo) {
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
      if(kilo.circleid && *(kilo.circleid) != DENSHA_SPECIAL_NONE)
      { ost << DENSHA_TYPE_LABEL[*(kilo.circleid)] << '\n'; }
      else if(kilo.denshaid && *(kilo.denshaid) != DENSHA_SPECIAL_NONE)
      { ost << DENSHA_TYPE_LABEL[*(kilo.denshaid)] << '\n'; }
      return ost;
    }
  };
}

#pragma once

#include <cstdlib>
#include <string>
#include <map>
#include <ostream>
#include <boost/preprocessor.hpp>

#define $ (*this)
#define PRT(x) BOOST_PP_STRINGIZE(x) << " " << x << " "

// Depends on Locale
namespace liquid
{
  namespace
  {
    // From wide to multibyte
    inline void wide2multi(const std::wstring & src, std::string & dest)
    {
      char * mbs = new char[src.length() * MB_CUR_MAX + 1];
      std::wcstombs(mbs, src.c_str(), src.length() * MB_CUR_MAX + 1);
      dest = mbs;
      delete [] mbs;
    }

    // From multibyte to wide
    inline void multi2wide(const std::string & src, std::wstring & dest)
    {
      wchar_t * wcs = new wchar_t[src.length() + 1];
      std::mbstowcs(wcs, src.c_str(), src.length() + 1);
      dest = wcs;
      delete [] wcs;
    }
  }

  /**
   * @~english
   * Interval Tree for ranges without overlap.
   */
  /**
   * @~japanese
   * 区間が重ならない前提の区間木.
   */
  template <class T>
  class UniqueIntervalTree
  {
  private:
    typedef typename std::map<T, T> Container;
    Container tree;

  public:
    typedef typename Container::iterator iterator;
    typedef typename Container::const_iterator const_iterator;

    /**
     * Clear all ranges.
     */
    void clear() {
      tree.clear();
    }

    /**
     * Dump all ranges.
     */
    friend std::ostream & operator<<(std::ostream & ost,
                                     const UniqueIntervalTree<T> & obj) {
      for(typename UniqueIntervalTree<T>::const_iterator i = obj.tree.begin();
          i != obj.tree.end(); ++i)
      {
        if(i != obj.tree.begin()) { ost << ","; }
        ost << "[" << i->first << "," << i->second <<  "]";
      }
      return ost;
    }

    /**
     * @~japanese
     * [begin, end]で指定した区間を挿入する.
     * @a begin と @a end が異なる必要がある.
     * @param[in] begin 区間の始点.
     * @param[in] end   区間の終点.
     * @retval true     挿入が成功した場合.
     * @retval false    挿入が重なりによって失敗した場合.
     */
    /**
     * @~english
     * Insert specified range [@a begin, @a end].
     * Constraint: @a begin != @a end
     * @param[in] begin Begin of the range.
     * @param[in] end   End of the range.
     * @retval true     Insertion succeed.
     * @retval false    Insertion failed due to overlapping.
     */
    bool insert(T begin, T end) {
      if(end < begin) { std::swap(begin, end); }
      if(!(begin < end)) { return false; }
      if(tree.empty())
      {
        std::pair<iterator, bool> ret =
          tree.insert(std::make_pair(begin, end));
        return ret.second;
      }
      const std::pair<iterator, iterator> range = tree.equal_range(begin);
      // all ranges are larger than begin.
      if(range.second == tree.begin())
      {
        if(tree.begin()->first < end) { return false; }
        tree.insert(tree.begin(), std::make_pair(begin, end));
        return true;
      }
      // all ranges are smaller than begin.
      if(range.first == tree.end())
      {
        iterator End = range.first;
        --End;
        if(begin < End->second) { return false; }
        tree.insert(End, std::make_pair(begin, end));
        return true;
      }
      iterator lower = range.second;
      --lower;
      const iterator upper = range.first;
      // constraint: example: [1,5] >= [5,9] =< [10,15]
      if(begin < lower->second || upper->first < end) { return false; }
      tree.insert(lower, std::make_pair(begin, end));
      return true;
    }

    /**
     * Check whether the point is in interval or not.
     * @param[in] point Point to query.
     * @retval true     When point is in ranges (including boudary.)
     * @retval false    When point is out of ranges
     */
    bool query(T point) {
      iterator itr = tree.upper_bound(point);
      if(itr == tree.begin()) return false;
      --itr;
      return itr->first <= point && point <= itr->second;
    }
  };
}

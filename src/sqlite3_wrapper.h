#pragma once

#include <iostream>
#include <exception>
#include <boost/utility.hpp>
#include <sqlite3.h>

namespace sqlite3_wrapper
{
  /**
   * IO Error Exception.
   */
  class IOException : std::exception
  {
  };

  /**
   * Wrapper object of sqlite3 object.
   */
  class SQLite : boost::noncopyable
  {
  private:
    sqlite3 * db;
  public:
    /**
     * Constructor with filname and options.
     * @param dbname[in] database filename.
     * @param flags[in] database file open mode.
     * @param zVfs[in] Name of VFS module to use. Refer SQLite document.
     */
    SQLite(const char * dbname,
	   int flags=SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
	   const char *zVfs=NULL)
    {
      int rc;
      rc = sqlite3_open_v2(dbname, &db, flags, zVfs);
      if(rc != SQLITE_OK)
	{
	  std::cerr << sqlite3_errmsg(db) << std::endl;
	  sqlite3_close(db);
	  throw IOException();
	}
    }
    ~SQLite()
    {
      sqlite3_close(db);
    }

    /**
     * Return rare pointer of sqlite3 object.
     * This is bad know-how.
     * @return pointer to sqlite3 object.
     */
    sqlite3 * ptr() const
    {
      return db;
    }

    /**
     * Return sqlite3 error message.
     * @return pointer to UTF-8 error message string.
     */
    const char * errmsg()
    {
      return sqlite3_errmsg(db);
    }
  };

  /**
   * Wrapper object of SQLite statement, sqlite3_stmt.
   */
  class SQLiteStmt : boost::noncopyable
  {
  private:
    sqlite3_stmt * stmt;
    SQLite & db;
  public:
    /**
     * Constructor with SQL statement string of char*.
     * @param db[in] SQLite object to tie up with.
     * @param query[in] query string of char *.
     * @param qlength[in] size of query in bytes.
     */
    SQLiteStmt(SQLite & db, const char * query, size_t qlength)
      : db(db)
    {
      sqlite3_prepare_v2(db.ptr(), query, qlength, &stmt, NULL);
      this->reset();
    }

    /**
     * Constructor with SQL statement string of std::string.
     * @param db[in] SQLite object to tie up with.
     * @param query[in] query string of std::string.
     */
    SQLiteStmt(SQLite & db, const std::string & query)
      : db(db)
    {
      sqlite3_prepare_v2(db.ptr(), query.c_str(), query.size(), &stmt, NULL);
      this->reset();
    }

    ~SQLiteStmt()
    {
      sqlite3_finalize(stmt);
    }

    /**
     * Function to bind integer value to place holder.
     * @param icol[in] place holder's column index starting from 1.
     * @param value[in] value to bind to place holder.
     * @return state of rare sqlite function.
     */
    int bind(int icol, int value)
    {
      return sqlite3_bind_int(stmt, icol, value);
    }

    int bind(int icol, double value)
    {
      return sqlite3_bind_double(stmt, icol, value);
    }

    int bind(int icol, const char * value, int vlength)
    {
      return sqlite3_bind_text(stmt, icol, value, vlength, SQLITE_STATIC);
    }

    int bind(int icol, const char * value, int vlength, void (*destructor)(void*))
    {
      return sqlite3_bind_text(stmt, icol, value, vlength, destructor);
    }

    int bind(int icol, const std::string & value)
    {
      return sqlite3_bind_text(stmt, icol, value.c_str(), value.size(), SQLITE_STATIC);
    }

    /**
     * Function to bind NULL to place holder.
     * @param icol[in] place holder's column index starting from 1.
     * @return state of rare sqlite function.
     */
    int bind_null(int icol)
    {
      return sqlite3_bind_null(stmt, icol);
    }

    /**
     * Function to get the number of place holders.
     * @return the number of place holders.
     */
    int bind_parameter_count()
    {
      return sqlite3_bind_parameter_count(stmt);
    }

    /**
     * Function to get place holder's index from its name.
     */
    int bind_parameter_index(const char *zName)
    {
      return sqlite3_bind_parameter_index(stmt, zName);
    }

    const char * bind_parameter_name(int n)
    {
      return sqlite3_bind_parameter_name(stmt, n);
    }

    /**
     * Function to clear all binded values.
     */
    int clear_bindings()
    {
      return sqlite3_clear_bindings(stmt);
    }

    /**
     * Class to convert from column index to various types.
     */
    struct column_value
    {
    private:
      const SQLiteStmt * t;
      int _icol;
    public:
      column_value(const SQLiteStmt * t, int icol) : t(t), _icol(icol){}
      operator int()
      {
	return sqlite3_column_int(t->stmt, _icol);
      }
      operator long long int()
      {
	return sqlite3_column_int64(t->stmt, _icol);	
      }
      operator const char *()
      {
	return reinterpret_cast<const char *>(sqlite3_column_text(t->stmt, _icol));
      }
      operator std::string()
      {
	return std::string(reinterpret_cast<const char *>(sqlite3_column_text(t->stmt, _icol)));
      }
    };

    /**
     * Function to get column value of specified index.
     */
    column_value column(int icol) const
    {
      return column_value(this, icol);
    }

    int step()
    {
      return sqlite3_step(stmt);
    }

    int reset()
    {
      return sqlite3_reset(stmt);
    }

    template <class T>
    bool fill_column(T & vec, int icol)
    {
      int rc;
      while((rc = this->step()) == SQLITE_ROW)
      {
        vec.push_back(this->column(icol));
      }
      if(rc != SQLITE_DONE)
      {
        std::cerr << db.errmsg() << std::endl;
        return false;
      }
      return true;
    }

    template <class T>
    bool fill_column(T & vec, int icol, int jcol)
    {
      int rc;
      while((rc = this->step()) == SQLITE_ROW)
      {
        vec.push_back(std::make_pair(this->column(icol),
                                     this->column(jcol)));
      }
      if(rc != SQLITE_DONE)
      {
        std::cerr << db.errmsg() << std::endl;
        return false;
      }
      return true;
    }
  };
}

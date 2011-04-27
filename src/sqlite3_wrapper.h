#pragma once

#include <iostream>
#include <exception>
#include <boost/tr1/memory.hpp>
#include <boost/utility.hpp>
#include <sqlite3.h>

namespace sqlite3_wrapper
{
  class IOException : std::exception
  {
  };

  class SQLite : boost::noncopyable
  {
  private:
    sqlite3 * db;
  public:
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
    sqlite3 * ptr() const
    {
      return db;
    }
    const char * errmsg()
    {
      return sqlite3_errmsg(db);
    }
  };

  class SQLiteStmt : boost::noncopyable
  {
  private:
    sqlite3_stmt * stmt;
  public:
    SQLiteStmt(SQLite & db, const char * query, size_t qlength)
    {
      sqlite3_prepare_v2(db.ptr(), query, qlength, &stmt, NULL);
      this->reset();
    }
    SQLiteStmt(SQLite & db, const std::string & query)
    {
      sqlite3_prepare_v2(db.ptr(), query.c_str(), query.size(), &stmt, NULL);
      this->reset();
    }
    ~SQLiteStmt()
    {
      sqlite3_finalize(stmt);
    }
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

    int bind_null(int icol)
    {
      return sqlite3_bind_null(stmt, icol);
    }

    int bind_parameter_count()
    {
      return sqlite3_bind_parameter_count(stmt);
    }

    int bind_parameter_index(const char *zName)
    {
      return sqlite3_bind_parameter_index(stmt, zName);
    }

    const char * bind_parameter_name(int n)
    {
      return sqlite3_bind_parameter_name(stmt, n);
    }

    int clear_bindings()
    {
      return sqlite3_clear_bindings(stmt);
    }

    struct column_value
    {
    private:
      const SQLiteStmt * t;
      int _icol;
    public:
      column_value(const SQLiteStmt * t, int icol) : _icol(icol), t(t){}
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
    };

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
  };
}

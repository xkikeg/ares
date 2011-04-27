#!/usr/bin/env python
import os
import re
import sys
import string
import sqlite3
from optparse import OptionParser

DATA_DIR = "data/"
#DB_NAME = ":memory:"
DB_NAME = "ares.sqlite"
DEBUG = False
#DEBUG = True
DEBUG_ALLROWS = False
#DEBUG_ALLROWS = True
TARGETS = (
    "company",
    "fare",
    "fare_country",
    "densha",
    "urban",
    "station",
    "line",
    "city",
    "kilo"
    )

class CSVParseException(Exception):
    pass

def typechange(x, typestring, isnull):
    null = "NULL" if isnull else 0
    if typestring == "text":
        return x.decode("utf-8")
    elif typestring == "integer":
        return int(x) if x != "" else null
    elif typestring == "trash":
        return None
    else:
        return x


def parse_csv(line):
    return tuple(string.strip(i, '"') for i in line[:-1].split(','))


# Read header.
# Header format is like that shown below.
# id:integer:p, name:text:un, other:integer::foo[name]
# Header is separated with comma.
# First one means 'id' column is integer and primary key.
# Second one means 'name' column is text and UNIQUE NOT NULL.
# Third one means 'other' column is integer and
# 'foo' table's 'name' column value is written.
# This program will find foo(name) and insert foo(id) value.
def parse_header(line):
    headline = [i.split(':') for i in parse_csv(line)]
    names = tuple(i[0] for i in headline)
    types = tuple(i[1] for i in headline)
    constraint = tuple(i[2] if len(i) > 2 else None for i in headline)
    foreignkey = tuple(re.match(r"(.*?)\[(.*?)\]", i[3]).groups()
                       if len(i) > 3 else None for i in headline)
    primarykey = tuple(col for col, opt in zip(names, constraint)
                       if opt is not None and string.find(opt, 'p') >= 0)
    options = tuple(((" NOT NULL" if string.find(opt, 'n') >= 0 else " ")+
                     (" UNIQUE"   if string.find(opt, 'u') >= 0 else " "))
                    if opt is not None else " "
                    for opt in constraint)
    return (names, types, constraint, foreignkey, primarykey, options)


def mktable_from_csv(db, tablename, filename = None):
    if DEBUG: print "Start Table: ", tablename
    if filename is None: filename = DATA_DIR + tablename + ".csv"
    # Open CSV file.
    csv = open(filename, 'r')
    # Read header.
    (names,
     types,
     constraint,
     foreignkey,
     primarykey,
     options) = parse_header(csv.readline())
    if DEBUG: print names, types, constraint, foreignkey, options

    # Read rows.
    rawrows = [parse_csv(i) for i in csv.readlines()]
    rows = []
    for row in rawrows:
        if row[0] == "": continue
        cols = []
        for i, col in enumerate(row):
            if foreignkey[i] is None:
                x = typechange(col, types[i],
                               constraint[i] is None or
                               string.find(constraint[i], 'n') == -1)
            elif string.upper(col) == "NULL" or col == "":
                x = "NULL"
            else:
                cur = db.execute("SELECT %sid FROM %s WHERE %s='%s'" %
                                 (foreignkey[i][0], foreignkey[i][0],
                                  foreignkey[i][1], col)
                                 ).fetchone()
                if cur is not None and len(cur) > 0: x = cur[0]
                else:
                    raise CSVParseException("value '%s' is not found in %s.%s"
                                            % (col,
                                               foreignkey[i][0],
                                               foreignkey[i][1]))
            if x is not None: cols.append(x if x != "NULL" else None)
        rows.append(tuple(cols))
    if DEBUG_ALLROWS:
        for row in rows:
            for i in row:
                print i, "@",
            print ""

    # Delete trash column.
    trash = lambda l: tuple(x for i,x in enumerate(l) if types[i] != "trash")
    names = trash(names)
    options = trash(options)
    types = trash(types)

    original_cols = names

    # Add primary key 'id' if there is not any primary key.
    if len(primarykey) == 0:
        names = (tablename+"id", ) + names
        types = ("INTEGER", ) + types
        constraint = ("p", ) + constraint
        foreignkey = (None, ) + foreignkey
        options = (" PRIMARY KEY AUTOINCREMENT", ) + options

    # Create DB.
    sql = ("CREATE TABLE %s(" % tablename +
           ",".join(i+" "+j+" "+k for i,j,k in zip(names, types, options)) +
           (", PRIMARY KEY (%s)" % ",".join(i for i in primarykey)
            if len(primarykey) > 0 else "") +
           "".join(", FOREIGN KEY(%s) REFERENCES %s(%sid)" %
                   (names[i], fkey[0], fkey[0])
                   for i, fkey in enumerate(foreignkey)
                   if fkey is not None) +
           ");")
    if DEBUG: print sql
    db.execute(sql)

    # Insert data.
    sql = ("INSERT INTO %s (%s) values (%s)" %
           (tablename,
            ",".join(i for i in original_cols),
            ",".join('?' for i in range(0, len(original_cols)))))
    if DEBUG: print sql
    #db.executemany(sql, rows)
    for row in rows:
        try:
            db.execute(sql, row)
        except:
            print >>sys.stderr, "Error occurred: ",
            for i in row:
                print i, "@",
            print ""
            raise


def mksqldb(db_name=DB_NAME):
    if os.path.exists(db_name): os.unlink(db_name)
    db = sqlite3.connect(db_name)
    db.execute("PRAGMA foreign_keys = ON;")
    for i in TARGETS:
        mktable_from_csv(db, i)
    db.commit()
    db.close()


def main():
    usage = "usage: %prog [-d database]"
    parser = OptionParser(usage)
    parser.add_option("-d", "--database", default=DB_NAME)
    (option, args) = parser.parse_args()
    mksqldb(option.database)


if(__name__ == "__main__"):
    main()

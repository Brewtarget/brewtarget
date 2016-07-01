#!/usr/bin/python

import sys
import json
import sqlite3

def addIng(cursor, x, tableName):
   columnList = x.keys()
   valueList = x.values()
   cvList = zip(columnList, valueList)
   
   setClause = ''
   for c,v in cvList[:-1]:
      setClause = setClause + '{0} = "{1}", '.format(c,v)
   setClause = setClause + '{0} = "{1}"'.format(cvList[-1][0], cvList[-1][1])
   
   cursor.execute('INSERT INTO {0} DEFAULT VALUES'.format(tableName))
   newId = cursor.lastrowid

   cursor.execute('UPDATE {0} SET {1} WHERE id = {2}'.format(tableName, setClause, newId))

   cursor.execute('INSERT INTO bt_{0} DEFAULT VALUES'.format(tableName))
   newBtId = cursor.lastrowid
   cursor.execute(
      'UPDATE bt_{0} '.format(tableName) +
      'SET {0}_id = '.format(tableName) + str(newId) + ' ' +
      'WHERE id = ' + str(newBtId)
   )

def addFerm(cursor, x):
   addIng(cursor, x, "fermentable")

def addHop(cursor, x):
   addIng(cursor, x, "hop")

def addMisc(cursor, x):
   addIng(cursor, x, "misc")

def addYeast(cursor, x):
   addIng(cursor, x, "yeast")

if __name__ == "__main__" :
   
   jsonFilename = sys.argv[1]
   databaseFilename = '../data/default_db.sqlite'
   dbConn = sqlite3.connect(databaseFilename)
   c = dbConn.cursor()
   
   with open(jsonFilename) as jsonFile:
      bigDict = json.load(jsonFile)
      ferm = bigDict['fermentable']
      for f in ferm:
         addFermentable(c,f)
      hop = bigDict['hop']
      for h in hop:
         addHop(c,h)
      misc = bigDict['misc']
      for m in misc:
         addMisc(c,m)
      yeast = bigDict['yeast']
      for y in yeast:
         addYeast(c, y)

   dbConn.commit()
   dbConn.close()
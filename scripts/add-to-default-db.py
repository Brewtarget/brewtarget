#!/usr/bin/python

import sys
import json
import sqlite3

class DbContext:
   def __init__(self, filename):
      self.dbconn = sqlite3.connect(filename)

   def __enter__(self):
      return self

   def __exit__(self, type, value, traceback):
      if type:
         print("Error occured while handling data, Rolling back any changes to the database:\n '{0}':'{1}'\n{2}".format(type, value, traceback))
         self.dbconn.rollback()
      else:
         self.dbconn.commit()
      self.dbconn.close()
      return True
   
   def addBatch(self, jsondict, table):
      for row in jsondict.get(table, []):
         self.addIng(row, table)

   def addIng(self, x, tableName):
      cursor = self.dbconn.cursor()
      setClause = ''
      for c, v in x.items():
         setClause = setClause + '{0} = "{1}", '.format(c,v)
      setClause = setClause[:-2]
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

if __name__ == "__main__" :
   jsonFilename = sys.argv[1]
   databaseFilename = '../data/default_db.sqlite'
   with open(jsonFilename) as jsonFile, DbContext(databaseFilename) as c:
      bigDict = json.load(jsonFile)
      c.addBatch(bigDict, 'fermentable')
      c.addBatch(bigDict, 'hop')
      c.addBatch(bigDict, 'misc')
      c.addBatch(bigDict, 'yeast')

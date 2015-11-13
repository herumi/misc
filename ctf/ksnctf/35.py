import sqlite3

conn = sqlite3.connect('35database.db')
c = conn.cursor()
c.execute('select * from user')
for r in c:
	print r

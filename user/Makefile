CC = gcc
MYSQLCFG = /usr/bin/mysql_config

WriteDB : WriteDB.o
	$(CC) -o WriteDB $(shell $(MYSQLCFG) --libs) WriteDB.o

WriteDB.o : WriteDB.c
	$(CC) $(shell $(MYSQLCFG) --cflags) -c WriteDB.c

clean:
	rm WriteDB.o WriteDB

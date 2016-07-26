CC = gcc
MYSQLCFG = /usr/bin/mysql_config

ConfigFilter : ConfigFilter.o ParseFeature.o
	$(CC) -o ConfigFilter $(shell $(MYSQLCFG) --libs) ConfigFilter.o ParseFeature.o

ConfigFilter.o : ConfigFilter.c
	$(CC) $(shell $(MYSQLCFG) --cflags) -c ConfigFilter.c

ParseFeature.o : ParseFeature.c 
	$(CC) -c ParseFeature.c

clean :
	rm ConfigFilter.o ParseFeature.o ConfigFilter

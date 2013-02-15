BASE = 
CC = g++
CFLAGS = -Wall -g \
		 -I$(BASE)/usr/include/php5 \
	     -I$(BASE)/usr/include/php5/main \
	     -I$(BASE)/usr/include/php5/Zend \
	     -I$(BASE)/usr/include/php5/TSRM \
	     -I$(BASE)/usr/include/mysql
LFLAGS = -lresolv -lcrypt -ldl -L$(BASE)/usr/lib -lphp5embedded

.cc.o:
	$(CC) $(CFLAGS) -c $< -o $*.o 

unserialize_php.so: unserialize_php.o pfc.o
	$(CC) $(LFLAGS) -Wall -fPIC -shared -o $@ unserialize_php.o pfc.o

test: test.o pfc.o
	$(CC) $(LFLAGS) test.o pfc.o -o $@

clean:
	@rm -f *.so test *.o

#
# $Log$
#

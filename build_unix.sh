cd bin/unix
#!/bin/bash
LIBNAME='cod4x_mysql.so'

echo Compiling...
	gcc -m32 -O3 -Wall -Wattributes -fPIC -s -fvisibility=hidden -I../../mysql/unix/include  -c ../../*.c

echo Linking...
	gcc -m32 -O3 -s -shared -fvisibility=hidden -o $LIBNAME *.o ../../mysql/unix/lib/libmysqlclient.a

echo Cleaning up...
	rm *.o

echo Done.

cd ../..

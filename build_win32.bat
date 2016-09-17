@echo off
cd bin/win32
set LIBNAME="cod4x_mysql.dll"

echo Compiling...
	gcc -O3 -Wall -s -fvisibility=hidden -I../../mysql/windows/include -c ../../*.c

echo Linking...
	gcc -O3 -s -shared -fvisibility=hidden -o %LIBNAME% *.o ../../mysql/windows/lib/libmysql.lib ../../../libcom_plugin.a

echo Cleaning up...
	del *.o

echo Done.

cd ../..

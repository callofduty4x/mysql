@echo off
cd bin/win32
set LIBNAME="cod4x_mysql.dll"

echo Compile...
	gcc -O3 -Wall -I../../mysql/windows/include -c ../../*.c

echo Link...
	gcc -O3 -shared -o %LIBNAME% *.o ../../mysql/windows/lib/libmysql.lib ../../../libcom_plugin.a

echo Cleanup...
	del *.o

echo Done.

cd ..

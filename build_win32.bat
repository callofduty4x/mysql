@echo off
cd bin

echo Compile...
	gcc -O3 -I../mysql/include -c ../*.c

echo Link...
	gcc -O3 -shared -o cod4x_mysql.dll *.o ../mysql/lib/libmysql.lib ../../libcom_plugin.a

echo Cleanup...
	del *.o

echo Done.

cd ..

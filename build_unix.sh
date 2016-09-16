cd bin
#!/bin/bash
NAME='mysql'

#Compiling: release
echo `gcc -m32 -Wall -O1 -s -fvisibility=hidden -mtune=core2  -I../mysql/unix/include  -c ../*.c`

#Linking
echo `gcc -m32 -s -shared -fvisibility=hidden -L../mysql/unix/lib -lmysqlclient -o $NAME''.so *.o`

#Cleaning up
echo `rm *.o`

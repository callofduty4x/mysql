##############################################
# Name of your plugin.                       #
# (output file: lib<PLUGIN_NAME>.so or .dll) #
##############################################
PLUGIN_NAME=cod4x_mysql
############################################
# List of files to be compiled into plugin #
############################################
PLUGIN_MODULES=plugin_main.c script_functions.c
#######################################################
# Header files include directory for Windows and UNIX #
#######################################################
INCLUDEDIR_UNIX=-Imysql/unix/include
INCLUDEDIR_WIN32=-Imysql/windows/include
#################################################
# Additional static libraries needs to be added #
#################################################
ADDITIONALLIBS_UNIX=mysql/unix/lib/libmysqlclient.a
ADDITIONALLIBS_WIN32=mysql/windows/lib/libmysql.lib




#################################
# Common definitions            #
# (rarely needed to be changed) #
#################################
CC=gcc
CFLAGS=-fvisibility=hidden -m32 -O3 -Wall -s --pedantic
PLUGIN_OBJECTS=$(patsubst %.c,obj/%.o,$(PLUGIN_MODULES))

TARGET_OS = $(shell $(CC) -dumpmachine)

ifeq      ($(TARGET_OS),mingw32)
INCLUDEDIR=$(INCLUDEDIR_WIN32)
ADDITIONALLIBS=$(ADDITIONALLIBS_WIN32)
else ifeq ($(TARGET_OS),cygwin)
else
CFLAGS += -fPIC
INCLUDEDIR=$(INCLUDEDIR_UNIX)
ADDITIONALLIBS=$(ADDITIONALLIBS_UNIX)
endif


############
# Recipies #
############
all:
	@echo "Use 'win32' or 'unix' recipe"

obj/%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDEDIR) -o $@ -c $<

clean:
	@echo "Use 'clean_win32' or 'clean_unix' recipe"


#####################
# Recipies: Windows #
#####################
win32: bin/lib$(PLUGIN_NAME).dll

bin/lib$(PLUGIN_NAME).dll: $(PLUGIN_OBJECTS)
	$(CC) -shared $(CFLAGS) -o $@ $^ $(ADDITIONALLIBS) ../libcom_plugin.a

clean_win32:
	del obj\*.o


##################
# Recipies: UNIX #
##################
unix: bin/lib$(PLUGIN_NAME).so

bin/lib$(PLUGIN_NAME).so: $(PLUGIN_OBJECTS)
	$(CC) -shared $(CFLAGS) -o $@ $^ $(ADDITIONALLIBS)

clean_unix:
	rm -r obj/*.o

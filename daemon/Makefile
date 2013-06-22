DESTDIR	=
prefix	= /usr/local
bindir	= $(prefix)/bin
sbindir	= $(prefix)/sbin
mandir	= $(prefix)/share/man
man8dir	= $(mandir)/man8
incdir	= $(prefix)/include

INSTALL		:= install
INSTALL_DATA	:= $(INSTALL) -m 644
INSTALL_DIR	:= $(INSTALL) -m 755 -d
INSTALL_PROGRAM	:= $(INSTALL) -m 755
RM		:= rm -f

CC	:= gcc

CFLAGS	?= -O2
CFLAGS	+= -Wall -Ilibbcm2835/src/
# When debugging, use the following instead
#CFLAGS	:= -O -g

LDFLAGS += -Llibbcm2835/src/ -L/usr/lib/arm-linux-gnueabihf
LIBS    += -lpthread -lmicrohttpd -lrt -l bcm2835  -lz -lm -ldl -lsqlite3

SOURCES=main.c dataserver.c data_handlers.c temp_reader.c control_output.c pid.c hardcode_settings.c log.c
HEADERS=ctrlr.h config.h
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=ctrlr

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) -c $< 

clean:
	rm -rf *~ $(OBJECTS) $(EXECUTABLE)

HEADERS = alarmclock_dbus_generated.h
OBJECTS = main.o alarmclock_dbus_generated.o
SOURCES = main.c alarmclock_dbus_generated.c
CC=gcc
CFLAGS+= -Wall -Werror -Wpointer-arith -fno-strict-aliasing
CFLAGS+=`pkg-config --cflags dbus-1 glib-2.0 gio-2.0 gthread-2.0 gio-unix-2.0 gobject-2.0`
LDFLAGS+=`pkg-config --libs dbus-1 glib-2.0 gio-2.0 gthread-2.0 gio-unix-2.0 gobject-2.0`

all: alarmclock

alarmclock : $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

$(OBJECTS) : $(SOURCES)
	$(CC) $(CFLAGS) -c ${@:.o=.c} -o $@  

#%.o: %.c
#	$(CC) $(CFLAGS) $< -o $@  $(LIBS)

#alarmclock: $(OBJECTS)
#	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LIBS)

clean:
	-rm -f $(OBJECTS)
	-rm -f dbus

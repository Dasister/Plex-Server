CC=gcc
CFLAGS=-c -Wall
LDFLAGS=-std=gnu99
SOURCES=main.c settings.c string.c
OBJECTS=$(SOURCES:.c=.o)
BUILDDIR=./build
EXECUTABLE=plexserver

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@


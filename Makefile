# Macro
TARGET = eval
CC = gcc-4.9
OBJS = eval.o encoder.o decoder.o measure.o
CFLAGS = -Wall -O2 -m64

INCLUDES = -I/usr/include -I/usr/include/isa-l -I/usr/local/include -I/usr/local/include/jerasure
LIBS = -L/usr/lib -L/usr/local/lib -lJerasure

# Rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LIBS) -o $@ $(OBJS) /usr/lib/libisal.a /usr/local/lib/libJerasure.la

clean:
	-rm -f $(TARGET) $(OBJS) ./encoded/* ./decoded/*

# suffix rurles
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

eval.o: eval.h


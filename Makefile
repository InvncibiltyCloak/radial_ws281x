CC = gcc
CFLAGS  = -g -Wall -Werror -std=gnu99
MAIN = main
INCLUDES = -I./include
LIBPATH = -L./lib
LIBS = -lturbojpeg -lws2811 -lm

SRCS = main.c bilinear.c
OBJS = $(SRCS:.c=.o)

all: build

build: $(MAIN)

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LIBPATH) $(LIBS)

%.o: %.c %.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm *.o *~ $(MAIN)

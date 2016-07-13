CC = gcc
CFLAGS  = -g -Wall
MAIN = main
LIBS = -lturbojpeg

SRCS = main.c bilinear.c
OBJS = $(SRCS:.c=.o)

all: build

build: $(MAIN)

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LIBS)

%.o: %.c %.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm *.o *~ $(MAIN)

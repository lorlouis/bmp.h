OBJS	= main.o bmp.o
SOURCE	= main.c bmp.c
HEADER	=
OUT	= a.out
CC	= gcc
FLAGS	= -g -c -Wall
LFLAGS	=

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

%.o: %.c
	$(CC) $(FLAGS) $^

clean:
	rm -f $(OBJS) $(OUT)


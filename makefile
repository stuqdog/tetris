CXX=gcc
CFLAGS=-Wall -g
LDLFLAGS=-L/usr/local/lib -lallegro -lallegro_main -lallegro_image -lallegro_font
INCLUDE=-I. -I/usr/local/lib


OBJS=main.c
OUT=main

all: hello_rule

clean:
		rm -f

hello_rule: $(OBJS)
	$(CXX) $(OBJS) -o $(OUT) $(INCLUDE) $(CFLAGS) $(LDLFLAGS)
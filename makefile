# filename          /makefile
# date              2018.11.06
# last edit date    2018.11.06
# author            NO.00[UNKNOWN]
# brief             makefile for ImageBuilder

all: ImageBuilder.exe

ImageBuilder.exe: ImageBuilder.c
	gcc -o $@ $<

clean:
	rm -f ImageMaker.exe
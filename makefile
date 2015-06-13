all:
	gcc -g -fno-omit-frame-pointer -std=c99 -c *.c
	gcc *.o -o a.out

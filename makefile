all:
	gcc -g -std=c99 -c *.c
	gcc *.o -o a.out

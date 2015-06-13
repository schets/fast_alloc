all:
	gcc -O3 -std=c99 -c -g *.c
	gcc *.o -o a.out

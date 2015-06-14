all:
	gcc -g -O3 -std=c99 -c *.c tests/*.c
	gcc *.o -o a.out

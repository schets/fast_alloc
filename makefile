all:
	gcc -g -std=c99 -c *.c tests/*.c
	gcc *.o -o a.out

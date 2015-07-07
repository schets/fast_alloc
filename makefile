all:
	gcc -g -O3 -std=c11 -c *.c tests/*.c
	gcc *.o -o a.out

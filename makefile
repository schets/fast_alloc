all:
	clang -O3 -g -std=c99 -c *.c tests/*.c
	clang *.o -o a.out

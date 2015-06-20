all:
	clang -g -O3 -std=c99 -c *.c tests/*.c
	clang *.o -o a.out

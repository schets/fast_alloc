all:
	gcc -g -O3 -flto -std=c11 -c *.c tests/*.c
	gcc *.o -flto -o a.out

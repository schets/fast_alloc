all:
	clang -g -fsanitize=address -fno-omit-frame-pointer -std=c99 -c *.c
	clang -fsanitize=address *.o -o a.out

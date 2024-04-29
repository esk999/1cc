#include <stdio.h>

int func0() { printf("you call func0()\n"); }
int func1(int i) { printf("you call func1(i)\n"); printf("i=%d\n", i); }
int func2(int x, int y) { printf("you call func2(x, y)\n"); printf("x=%d\n", x); printf("y=%d\n", y); printf("x+y=%d\n", x + y); }
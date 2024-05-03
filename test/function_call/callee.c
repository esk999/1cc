#include <stdio.h>
#include <malloc.h>

int alloc4(int **ptr, int i0, int i1, int i2, int i3){
  *ptr = (int *)malloc(4 * sizeof(int));
  (*ptr)[0] = i0;
  (*ptr)[1] = i1;
  (*ptr)[2] = i2;
  (*ptr)[3] = i3;
  return 0;
}

int print(int i){
  printf("%d\n", i);
}

int func0(){
  printf("you call func0()\n");
}

int func1(int i){
  printf("you call func1(i)\n");
  printf("i=%d\n", i);
}

int func2(int x, int y){
  printf("you call func2(x, y)\n");
  printf("x=%d\n", x);
  printf("y=%d\n", y);
  printf("x+y=%d\n", x + y);
}
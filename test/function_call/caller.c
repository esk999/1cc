#include <stdio.h>


void func0();
void func1(int x);
void func2(int x, int y);
int print(int i);
int alloc4(int **ptr, int i0, int i1, int i2, int i3);

int main(int argc, char const *argv[])
{
  func0();
  func1(5);
  func2(2, 3);
  printf("alloc4\n");
  int *ptr;
  int ret = alloc4(&ptr, 100, 101, 102, 103);
  print(ptr[0]);
  print(ptr[1]);
  print(ptr[2]);
  print(ptr[3]);
  return 0;
}
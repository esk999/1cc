#!/bin/bash
runTest() {
    ./1cc "tests/test.c" > tmp.s
    cd func
    cc -c func.c
    cd ..
    cc -static -o tmp tmp.s func/func.o
    ./tmp
}
runTest
# # string
# assert 97 '
# int main(){
#   char *a;
#   a = "abcd"; // テスト
#   printf(a);
#   return a[0];
# }
# '

# # global
# assert 10 "
# int a;
# int b[10];
# int main(){
#   a = 10;
#   return a;
# }
# "
# assert 2 "int main() {return 2;}"
# assert 2 "int main() return 2;"
# assert 3 "
# int main(){return func(1, 2);}
# int func(int a, int b) return a + b;
# "
# assert 4 "
# int main(){return func(1, 2, 3);}
# int func(int a, int b, int c) return a + c;
# "
# assert 55 "int main(){
# int a; a=10; return total(a);} 
# int total(int n){ if(n<0)return 0; return n + total(n-1);}
# "
# assert 42 "int main()return 42;"
# assert 21 "int main()return 5+20-4;"
# assert 41 "int main()return  12 + 34 - 5 ;"
# assert 47 'int main()return 5+6* 7;'
# assert 15 'int main()return 5*(9-6);'
# assert 4 'int main()return (3+5)/2;'
# assert 10 "int main()return -10+20;"
# assert 0 "int main()return 0==1;"
# assert 1 "int main()return 42==42;"
# assert 1 "int main()return 0!=1;"
# assert 0 "int main()return 42!=42;"

# assert 1 "int main()return 0<1;"
# assert 0 "int main()return 1<1;"
# assert 0 "int main()return 2<1;"
# assert 1 "int main()return 0<=1;"
# assert 1 "int main()return 1<=1;"
# assert 0 "int main()return 2<=1;"

# assert 1 "int main()return 1>0;"
# assert 0 "int main()return 1>1;"
# assert 0 "int main()return 1>2;"
# assert 1 "int main()return 1>=0;"
# assert 1 "int main()return 1>=1;"
# assert 0 "int main()return 1>=2;"

# assert 5 "int main(){int a; a=2;int b; b=a;1+2*b;}"
# assert 5 "int main(){int n; n=2; int num; num=n; 1+2*num;}"
# assert 4 "int main(){int n; n=1; int m; m=3; n+m;}"
# assert 5 "int main(){int bar; bar=2+3; return bar;}"
# assert 6 "int main(){1 + (2+3);}"
# assert 6 "int main(){int foo; foo=1; int bar; bar=2+3; foo + bar;}"
# assert 33 "int main(){int b; b=1; b=2; int c; c=31; b+c;}"
# assert 4 "int main(){return 4; 1; 2;}"
# assert 5 "int main(){1; return 5; 3;}"
# assert 4 "int main(){1;5;return 4;}"
# assert 6 "int main(){int foo; foo=1; int bar; bar=2+3; return foo + bar;}"
# assert 4 "int main(){int a; a=1; if(a==1) return 4;}"
# assert 3 "int main(){int a; a=0; if(a!=1) return 3;}"
# assert 3 "int main(){int a; a=1; int b; b=2; if(a<b) return 3;}"
# assert 5 "int main(){int a; a=2; int b; b=3; if(b>=a) a+b;}"
# assert 5 "int main(){int a; a=1; if(a==2) return 4; else return 5;}"
# assert 6 "int main(){int a; a=1; if(a!=1) return 4; else if(a==1) return 6; else return 3;}"
# assert 5 "int main(){int a; a=2; int b; b=3; if(b>=a) a+b; else if(b<a) return 4; else return 1;}"
# assert 10 "int main(){int a; a=0; while(a<10) a=a+1; return a;}"
# assert 10 "int main(){int a; a=0; int i; for(i=0; i<10; i=i+1) a = a + 1; return a;}"
# assert 100 "int main(){ int a; a=0; int i; for(i=0; i<10; i=i+1) a = a + 1; return a*i;}"

# # #========
# # # block
# assert 15 "
# int main(){
# int x;
# x=0;
# int i;
# for(i=0;i<10;i=i+1)
# {
#   if (i<5)
#   {
#     x=x+1;
#   }
#   else
#   {
#     x=x+2;
#   }
# }
# return x;
# }"

# assert 1 "
# int main(){
# int x;
# x=0;
# int i;
# for(i;i<10;i=i+1)
# {
#   int a;
#   a=1;
#   if (i<5)
#   {
#     x=x+1;
#   }
#   else
#   {
#     x=x+2;
#   }
# }
# return a;
# }"

# assert 17 "
# int main(){
# int x;
# x=0;
# int i;
# for(i=0;i<10;i=i+1)
# {
#   int j;
#   j=i*2;
#   if (j<5)
#   {
#     x=x+1;
#   }
#   else
#   {
#     x=x+2;
#   }
#   j=0;
# }
# return x;
# }
# "

# # func
# assert 1 "int main()return foo();"
# assert 7 "int main ()return bar(3, 4);"
# assert 12 "int main ()return bar2(3, 4, 5);"

# # * &
# assert 3 "
# int main(){
#   int x;
#   x = 3;
#   int *y;
#   y = &x;
#   return *y;
# }"

# assert 3 "
# int main(){
#   int x;
#   x = 3;
#   int y;
#   y = 5;
#   int *z;
#   z = &y + 8;
#   return *z;
# }"

# assert 2 "
# int main() { int x; x = 2; return x;}
# "

# assert 3 "
# int main(){
#   int x;
#   int *y;
#   y = &x;
#   *y = 3;
#   return x;
# }
# "

# assert 4 "
# int main(){
#   int *p;
#   alloc4(&p, 1, 2, 4, 8);
#   int *q;
#   q = p + 2;
#   return *q;
# }
# "

# assert 8 "
# int main(){
#   int *p;
#   alloc4(&p, 1, 2, 4, 8);
#   int *q;
#   q = p + 3;
#   return *q;
# }
# "

# assert 2 "
# int main(){
#   int *p;
#   alloc4(&p, 1, 2, 4, 8);
#   int *q;
#   q = p + 3;
#   q = q - 2;
#   return *q;
# }
# "
# # sizeof
# assert 4 "
# int main(){
#   int x;
#   return sizeof(x);
# }
# "

# assert 8 "
# int main(){
#   int *x;
#   return sizeof(x);
# }
# "
# assert 4 "
# int main(){
#   return sizeof(1);
# }
# "

# assert 8 "
# int main(){
#   int *x;
#   return sizeof(x + 3);
# }
# "

# assert 1 "
# int main(){
#   char x;
#   return sizeof(x);
# }
# "

# # array
# assert 0 "
# int main(){
#   int a[10];
#   return 0;
# }
# "

# assert 0 "
# int main(){
#   int a[10][20];
#   return 0;
# }
# "

# assert 3 "
# int main(){
#   int a[2];
#   *a = 1;
#   *(a + 1) = 2;
#   int *p;
#   p = a;
#   return *p + *(p + 1);
# }
# "

# assert 3 "
# int main(){
#   int a[2];
#   a[0] = 1;
#   a[1] = 2;
#   int *p;
#   p = a;
#   return p[0] + p[1];
# }
# "

# #char
# assert 3 "
# int main(){
#   char x[3];
#   x[0] = -1;
#   x[1] = 2;
#   int y;
#   y = 4;
#   return x[0] + y;
# }
# "
# echo OK
#!/bin/bash
assert(){
    expected="$1"
    input="$2"

    ./1cc "$input" > tmp.s 2> tmp.err
    if [ ! -s tmp.s ]; then
        echo "Compiler error or no output generated:"
        cat tmp.err
        exit 1
    fi

    ./1cc "$input" > tmp.s
    cd func
    cc -c func.c
    cd ..
    cc -o tmp tmp.s func/func.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 2 "main() {return 2;}"
assert 2 "main() return 2;"
assert 3 "
main(){return func(1, 2);}
func(a, b) return a + b;
"
assert 4 "
main(){return func(1, 2, 3);}
func(a, b, c) return a + c;
"
assert 55 "
main(){a=10; return total(a);}
total(n){ if(n<0)return 0; return n + total(n-1);}
"
assert 42 "main()return 42;"
assert 21 "main()return 5+20-4;"
assert 41 "main()return  12 + 34 - 5 ;"
assert 47 'main()return 5+6* 7;'
assert 15 'main()return 5*(9-6);'
assert 4 'main()return (3+5)/2;'
assert 10 "main()return -10+20;"
assert 0 "main()return 0==1;"
assert 1 "main()return 42==42;"
assert 1 "main()return 0!=1;"
assert 0 "main()return 42!=42;"

assert 1 "main()return 0<1;"
assert 0 "main()return 1<1;"
assert 0 "main()return 2<1;"
assert 1 "main()return 0<=1;"
assert 1 "main()return 1<=1;"
assert 0 "main()return 2<=1;"

assert 1 "main()return 1>0;"
assert 0 "main()return 1>1;"
assert 0 "main()return 1>2;"
assert 1 "main()return 1>=0;"
assert 1 "main()return 1>=1;"
assert 0 "main()return 1>=2;"

assert 5 "main(){a=2;b=a;1+2*b;}"
assert 5 "main(){n=2; num=n; 1+2*num;}"
assert 4 "main(){n=1; m=3; n+m;}"
assert 5 "main(){bar=2+3; return bar;}"
assert 6 "main(){1 + (2+3);}"
assert 6 "main(){foo=1; bar=2+3; foo + bar;}"
assert 33 "main(){b=1; b=2; c=31; b+c;}"
assert 4 "main(){return 4; 1; 2;}"
assert 5 "main(){1; return 5; 3;}"
assert 4 "main(){1;5;return 4;}"
assert 6 "main(){foo=1; bar=2+3; return foo + bar;}"
assert 4 "main(){a=1; if(a==1) return 4;}"
assert 3 "main(){a=0; if(a!=1) return 3;}"
assert 3 "main(){a=1; b=2; if(a<b) return 3;}"
assert 5 "main(){a=2; b=3; if(b>=a) a+b;}"
assert 5 "main(){a=1; if(a==2) return 4; else return 5;}"
assert 6 "main(){a=1; if(a!=1) return 4; else if(a==1) return 6; else return 3;}"
assert 5 "main(){a=2; b=3; if(b>=a) a+b; else if(b<a) return 4; else return 1;}"
assert 10 "main(){a=0; while(a<10) a=a+1; return a;}"
assert 10 "main(){a=0; for(i=0; i<10; i=i+1) a = a + 1; return a;}"
assert 100 "main(){ a=0; for(i=0; i<10; i=i+1) a = a + 1; return a*i;}"

#========
# block
assert 15 "
main(){
x=0;
for(i=0;i<10;i=i+1)
{
  if (i<5)
  {
    x=x+1;
  }
  else
  {
    x=x+2;
  }
}
return x;
}"

assert 1 "
main(){
x=0;
for(i=0;i<10;i=i+1)
{
  a=1;
  if (i<5)
  {
    x=x+1;
  }
  else
  {
    x=x+2;
  }
}
return a;
}"

assert 17 "
main(){
x=0;
for(i=0;i<10;i=i+1)
{
  j=i*2;
  if (j<5)
  {
    x=x+1;
  }
  else
  {
    x=x+2;
  }
  j=0;
}
return x;
}
"

# func
assert 1 "main()return foo();"
assert 7 "main ()return bar(3, 4);"
assert 12 "main ()return bar2(3, 4, 5);"

# * &
assert 3 "
main(){
  x = 3;
  y = &x;
  return *y;
}"

assert 3 "
main(){
  x = 3;
  y = 5;
  z = &y + 8;
  return *z;
}"

echo OK
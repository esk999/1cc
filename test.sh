#!/bin/bash
assert(){
    expected="$1"
    input="$2"

    ./1cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 47 '5+6* 7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 10 "-10+20;"
assert 0 "0==1;"
assert 1 "42==42;"
assert 1 "0!=1;"
assert 0 "42!=42;"

assert 1 "0<1;"
assert 0 "1<1;"
assert 0 "2<1;"
assert 1 "0<=1;"
assert 1 "1<=1;"
assert 0 "2<=1;"

assert 1 "1>0;"
assert 0 "1>1;"
assert 0 "1>2;"
assert 1 "1>=0;"
assert 1 "1>=1;"
assert 0 "1>=2;"

assert 5 "a=2;b=a;1+2*b;"
assert 5 "n=2; num=n; 1+2*num;"
assert 4 "n=1; m=3; n+m;"
assert 5 "bar=2+3;"
assert 6 "1 + (2+3);"
assert 6 "foo=1; bar=2+3; foo + bar;"
assert 33 "b=1; b=2; c=31; b+c;"
assert 4 "return 4; 1; 2;"
assert 5 "1; return 5; 3;"
assert 4 "1;5;return 4;"
assert 6 "foo=1; bar=2+3; return foo + bar;"
assert 4 "a=1; if(a==1) return 4;"
assert 3 "a=0; if(a!=1) return 3;"
assert 3 "a=1; b=2; if(a<b) return 3;"
assert 5 "a=2; b=3; if(b>=a) a+b;"
assert 5 "a=1; if(a==2) return 4; else return 5;"
assert 6 "a=1; if(a!=1) return 4; else if(a==1) return 6; else return 3;"
assert 5 "a=2; b=3; if(b>=a) a+b; else if(b<a) return 4; else return 1;"
assert 10 "a=0; while(a<10) a=a+1; return a;"
assert 10 "a=0; for(i=0; i<10; i=i+1) a = a + 1; return a;"
assert 100 "a=0; for(i=0; i<10; i=i+1) a = a + 1; return a*i;"

echo OK
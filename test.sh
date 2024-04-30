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

# assert 0 "main(){return 0;};"
# assert 42 "main(){return 42;};"
# assert 21 "main(){return 5+20-4;};"
# assert 41 "main(){return 12  + 34 - 5;};"
# assert 47 'main(){return 5+6* 7;};'
# assert 15 'main(){return 5*(9-6);};'
# assert 4 'main(){return (3+5)/2;};'
# assert 10 "main(){return -10+20;};"
# assert 0 "main(){return 0==1;};"
# assert 1 "main(){return 42==42;};"
# assert 1 "main(){return 0!=1;};"
# assert 0 "main(){return 42!=42;};"

# assert 1 "main(){return 0<1;};"
# assert 0 "main(){return 1<1;};"
# assert 0 "main(){return 2<1;};"
# assert 1 "main(){return 0<=1;};"
# assert 1 "main(){return 1<=1;};"
# assert 0 "main(){return 2<=1;};"

# assert 1 "main(){return 1>0;};"
# assert 0 "main(){return 1>1;};"
# assert 0 "main(){return 1>2;};"
# assert 1 "main(){return 1>=0;};"
# assert 1 "main(){return 1>=1;};"
# assert 0 "main(){return 1>=2;};"

# assert 5 "main(){a=2;b=a;1+2*b;};"
# assert 5 "main(){n=2; num=n; 1+2*num;};"
# assert 4 "main(){n=1; m=3; n+m;};"
# assert 5 "main(){bar=2+3; return bar;};"
# assert 6 "main(){1 + (2+3);};"
# assert 6 "main(){foo=1; bar=2+3; foo + bar;};"
# assert 33 "main(){b=1; b=2; c=31; b+c;};"
# assert 4 "main(){return 4; 1; 2;};"
# assert 5 "main(){1; return 5; 3;};"
# assert 4 "main(){1;5;return 4;};"
# assert 6 "main(){foo=1; bar=2+3; return foo + bar;};"
# assert 4 "main(){a=1; if(a==1) return 4;};"
# assert 3 "main(){a=0; if(a!=1) return 3;};"
# assert 3 "main(){a=1; b=2; if(a<b) return 3;};"
# assert 5 "main(){a=2; b=3; if(b>=a) a+b;};"
# assert 5 "main(){a=1; if(a==2) return 4; else return 5;};"
# assert 6 "main(){a=1; if(a!=1) return 4; else if(a==1) return 6; else return 3;};"
# assert 5 "main(){a=2; b=3; if(b>=a) a+b; else if(b<a) return 4; else return 1;};"
# assert 10 "main(){a=0; while(a<10) a=a+1; return a;};"
# assert 10 "main(){a=0; for(i=0; i<10; i=i+1) a = a + 1; return a;};"
# assert 100 "main(){a=0; for(i=0; i<10; i=i+1) a = a + 1; return a*i;};"
# assert 10 "main(){num1 = 1; num2 = 4; num3 = 2; return (num1 + num2) * num3;};"
# #========
# # block
# assert 15 "
# main(){
# x=0;
# for(i=0;i<10;i=i+1){
#   if (i<5){
#     x=x+1;
#   }
#   else{
#     x=x+2;
#   }
# }
# return x;
# };
# "

# assert 1 "
# main(){
# x=0;
# for(i=0;i<10;i=i+1){
#   a1=1;
#   if (i<5){
#     x=x+1;
#   }
#   else{
#     x=x+2;
#   }
# }
# return a1;
# };
# "
# assert 17 "
# main(){
# x=0;
# for(i=0;i<10;i=i+1)
# {
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
# };
# "

# assert 1 "
# foo(){
#   return 1;
# };
# main(){
#   return foo();
# };
# "

# assert 17 "
# main(){
#     x=0;
#     for(i=0;i<10;i=i+1){
#         j=i*2;
#         if (j<5){
#             x=x+1;
#         }
#         else{
#             x=x+2;
#         }
#         j=0;
#     }
#     return x;
# };
# "

# assert 3 "
# minus(i,j){
#     return i-j;
# };
# main(){
#     return minus(9,6);
# };"

# # ================
# # fibonacci
# input="
# fibonacci(n)
# {
#     if (n==0){
#         return n;
#     } else if (n==1){
#         return n;
#     } else{
#         return fibonacci(n - 2) + fibonacci(n - 1);
#     }
# };
# main(){
#     for(i=0;i<10;i=i+1){
#         print(fibonacci(i));
#     }
#     return 0;
# };"

# ./1cc "$input" > tmp.s
# gcc -o tmp tmp.s test/function_call/callee.s  
# printf "$input => "
# ./tmp

assert 3 "
main(){
x = 3;
y = &x;
return *y; 
};
"

assert 3 "
main(){
x = 3;
y = 5;
z = &y + 8;
return *z;
};
"
echo OK
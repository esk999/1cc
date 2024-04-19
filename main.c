#include "1cc.h"
int main(int argc, char **argv){
    if(argc != 2){
        error("%s: invalid number of arguments", argv[0]);
        return 1;
    }
    //トークナイズしてパースする
    user_input = argv[1];
    token = tokenize(user_input);
    Node *node = expr();
    

    //アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    //抽象構文木を下りながらコードを生成
    gen(node);

    //スタックトップに式全体の値が残っている
    //それをRAXにロードして関数からの帰り値とする
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}
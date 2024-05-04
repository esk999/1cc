#include "1cc.h"
int main(int argc, char **argv){
    if(argc != 2){
        error("%s: 引数の個数が正しくありません", argv[0]);
        return 1;
    }
    //現在注目しているトークン
    user_input = argv[1];
    //トークナイズする
    tokenize(user_input);
    //パースする
    program();

    //アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    //先頭の式から順にコードを生成
    cur_func=0;
    for(int i=0; code[i]; i++){
        gen(code[i]);
        cur_func++;
    }
    return 0;
}
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
    //label_indexを初期化
    label_index = 0;
    

    //アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");

    //先頭の式から順にコードを生成
    for(int i=0; code[i]; i++){
        gen(code[i]);
        // 式の評価結果としてスタックに1つの値が残っている
        // はずなので、スタックが溢れないようにポップしておく
        printf("    pop rax\n");
    }
    return 0;
}
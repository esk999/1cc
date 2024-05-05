#include "1cc.h"
int main(int argc, char **argv){
    if(argc != 2){
        error("%s: 引数の個数が正しくありません", argv[0]);
        return 1;
    }
    //現在注目しているトークン
    filename = argv[1];
    user_input = read_file(filename);
    //トークナイズする
    tokenize(user_input);
    //パースする
    program();

    //アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".bss\n");   // データセグメント
    // グローバル変数を先に生成
    for(int i=0; code[i]; i++){  
        if(code[i]->kind == ND_GVAR_DEF){
            gen(code[i]);  
        }   
    }
    printf(".data\n");
    for(StringToken *s = strings; s; s = s->next){
        printf(".LC_%d:\n", s->index);
        printf("    .string \"%s\"\n", s->value);
    }
    printf(".text\n");  // この直後から機械語にされる実行文を表す
    cur_func = 0;
    //先頭の式から順にコードを生成
    for(int i=0; code[i]; i++){
        if(code[i]->kind == ND_FUNC_DEF){
            cur_func++;
            gen(code[i]);  
        }
    }
    return 0;
}
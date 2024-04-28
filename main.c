#include "1cc.h"
int main(int argc, char **argv){
    if(argc != 2){
        error("%s: 引数の個数が正しくありません", argv[0]);
        return 1;
    }
    //現在注目しているトークン
    user_input = argv[1];
    //ローカル変数格納用の変数の初期設定
    locals = calloc(1, sizeof(LVar));
    locals->next = NULL;
    //トークナイズする
    tokenize(user_input);
    //パースする
    program();
    //label_indexを初期化
    label_index = 0;
    

    //アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    //プロローグ
    //変数26個分の領域を確保する
    printf("    push rbp\n");     // 現在のベースポインタの値をスタックにプッシュ
    printf("    mov rbp, rsp\n"); // スタックポインタをrbpにコピー
    printf("    sub rsp, 208\n"); // 26 * 8 = 208

    //先頭の式から順にコードを生成
    for(int i=0; code[i]; i++){
        gen(code[i]);
        // 式の評価結果としてスタックに1つの値が残っている
        // はずなので、スタックが溢れないようにポップしておく
        printf("    pop rax\n");
    }

    //エピローグ
    //最後の式の結果がRAXに残っているのでそれが返り値になる
    printf("    mov rsp, rbp\n"); // ベースポインタの値をrspに移動
    printf("    pop rbp\n");      // スタックから値をポップし，rbpに格納
    printf("    ret\n");          // スタックからリターンアドレスをポップする　呼び出し位置に戻る
    return 0;
}
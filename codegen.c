#include "1cc.h"

void gen_val(Node *node){
    if(node->kind == ND_DEREF){
        gen(node->lhs);
        return;
    }
    if(node->kind == ND_LVAR){
        printf("    mov rax, rbp\n");               // ベースポインタをraxに移動
        printf("    sub rax, %d\n", node->offset);  // raxが変数のメモリアドレスを示す
        printf("    push rax\n");                   // ローカル変数のアドレスをスタックにプッシュ
    }
    else if(node->kind == ND_GVAR){
        printf("    push offset %s\n", node->varname);
    }
    else{
        error("変数ではありません");
    }
}

//label_indexを初期化
int label_index = 0;

// レジスタ名定義
static char *argreg1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static char *argreg2[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
static char *argreg4[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static char *argreg8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen(Node *node){
    if (!node) return;
    label_index += 1;
    int id = label_index;
    int argcnt = 0;
    Type *t;
    switch(node->kind){
        case ND_STRING:
            printf("    push offset .LC_%d\n", node->string->index);
            return;

        case ND_NUM:
            printf("    push %d\n", node->val); // ノードの値をスタックにプッシュ
            return;
                
        case ND_LVAR:
        case ND_GVAR:
            gen_val(node);                     // ローカル変数のアドレスをスタックにプッシュ
            t = get_type(node);
            if(t && t->ty == ARRAY){
                return;                         // 配列の場合はアドレスをそのままにしたい
            }
            printf("    pop rax\n");            // ローカル変数のアドレスをraxレジスタに格納
            if(t && t->ty == CHAR){
                printf("    movsx rax, BYTE PTR [rax]\n");
            }
            else if(t && t->ty == INT){
                printf("    movsxd rax, DWORD PTR [rax]\n");
            }
            else{
                printf("    mov rax, [rax]\n");     // raxのアドレスにある値をraxに格納
            }
            printf("    push rax\n");           // スタックにプッシュ
            return;

        case ND_ASSIGN:
            gen_val(node->lhs);                // 左辺のローカル変数のアドレスをスタックにプッシュ
            gen(node->rhs);                     // 右辺の値をスタックにプッシュ
            t = get_type(node);
            printf("    pop rdi\n");            // 右辺の値をrdiに格納
            printf("    pop rax\n");            // 左辺のアドレスをraxに格納
            if(t && t->ty == CHAR){
                printf("    mov [rax], dil\n");
            }
            else if(t && t->ty == INT){
                printf("    mov [rax], edi\n");
            }
            else{
                printf("    mov [rax], rdi\n");     // 左辺のアドレスに右辺の値を格納
            }
            printf("    push rdi\n");           // 代入された値をスタックにプッシュ
            return;
                
        case ND_RETURN:
            gen(node->lhs);                     // return文の戻り値を表す式のアドレスをスタックにプッシュ
            printf("    pop rax\n");            // 戻り値を表す式のアドレスをraxレジスタに格納
            printf("    mov rsp, rbp\n");       // ベースポインタの値をスタックポインタの値に格納
            printf("    pop rbp\n");            // 前の関数のrbpの値を復元
            printf("    ret\n");                // 現在の関数から呼び出し元の関数に戻る
            return;

        case ND_IF:
            // if(node->condeition) node->lhsとなっている
            // 先にnode->condeitionのコードを作る
            gen(node->condition);                                      // ifの条件のアドレスをスタックにプッシュ
            // node->conditionのアドレスがスタックトップに残っているのでポップする
            printf("    pop rax\n");
            // raxと0の値を比べる
            printf("    cmp rax, 0\n");
            // raxの値が0ならifの条件は偽なので，node->lhsの処理は行わない
            printf("    je .Lend%03d\n", id);         // ゼロフラグが立っていれば処理終わりラベルにジャンプ
            // ジャンプしなかった場合，node->lhsのコードを作る
            gen(node->lhs);
            printf(".Lend%03d:\n", id);               // ジャンプ先（処理終わりラベル）
            return;

        case ND_IFELSE:
            // if(node->condtion) node->lhs else node->afetrthoughtとなっている
            // 先にnode->conditionのコードを作る
            gen(node->condition);                                      // ifの条件のアドレスをスタックにプッシュ
            // node->conditionのアドレスがスタックトップに残っているのでポップする
            printf("    pop rax\n");
            // raxと0の値を比べる
            printf("    cmp rax, 0\n");
            // raxの値が0ならifの条件は偽なので，node->lhsの処理は行わず，else文にジャンプ
            printf("    je .Lelse%03d\n", id);        // ゼロフラグが立っていれば.Lelseラベルにジャンプ
              
            // ジャンプしなかった場合，ifの処理であるnode->lhsのコードを作る
            gen(node->lhs);
            // if文の処理をしたら，else文の処理であるnode->afterthoughtの処理は行わない
            printf("    jmp .Lend%03d\n", id);
               
            // else文の処理　node->afterthoughtのコードを作る
            printf(".Lelse%03d:\n", id);               // ジャンプ先（.Lelseラベル）
            gen(node->afterthought);
                
            printf(".Lend%03d:\n", id);               // ジャンプ先（処理終わりラベル）
            return;

        case ND_WHILE:
            // while(node->condition) node->lhsとなっている
            // while文の始まりのラベル
            printf(".Lbegin%03d:\n", id);
            // node->conditionのコードを作る
            gen(node->condition);
            // node->conditionのアドレスがスタックトップに残っているのでポップする
            printf("    pop rax\n");
            // raxと0の値を比べる
            printf("    cmp rax, 0\n");
            // raxの値が0ならwhileの条件は偽なので，node->lhsの処理は行わず，終わりラベルにジャンプ
            printf("    je .Lend%03d\n", id);        // ゼロフラグが立っていれば.Lendラベルにジャンプ
              
            // ジャンプしなかった場合，whileの処理であるnode->lhsのコードを作る
            gen(node->lhs);
            // ループするために開始ラベルにジャンプする
            printf("    jmp .Lbegin%03d\n", id);
            printf(".Lend%03d:\n", id);              // ジャンプ先（処理終わりラベル）
            return;

        case ND_FOR:
            // for(node->initialize; node->condition; node->afterthought) node->lhsとなっている
            // 先にnode->initializeのコードを作る
            gen(node->initialize);
            // for文の始まりのラベル
            printf(".Lbegin%03d:\n", id);
            // node->conditionのコードを作る
            gen(node->condition);
            if (!node->condition){
                printf("  push 1\n");
            }
            // node->conditionのアドレスがスタックトップに残っているのでポップする
            printf("    pop rax\n");
            // raxと0の値を比べる
            printf("    cmp rax, 0\n");
            // raxの値が0ならwhileの条件は偽なので，node->lhsの処理は行わず，終わりラベルにジャンプ
            printf("    je .Lend%03d\n", id);        // ゼロフラグが立っていれば.Lendラベルにジャンプ
               
            // node->lhsのコードを作る　処理を先に行う
            gen(node->lhs);
            // node->afterthougtのコードを作る
            gen(node->afterthought);
              
            // ループするために開始ラベルにジャンプする
            printf("    jmp .Lbegin%03d\n", id);

            printf(".Lend%03d:\n", id);              // ジャンプ先（処理終わりラベル）
            return;

        case ND_BLOCK:
            for(int i = 0; node->block[i]; i++){
                gen(node->block[i]);
            }
            return;

        case ND_FUNC_CALL:
            for (int i = 0; node->block[i]; i++){
                gen(node->block[i]);
                argcnt++;
            }
            for(int i = argcnt - 1; i >= 0; i--){
                printf("    pop %s\n", argreg8[i]);
            }
            // RSPは16の倍数
            printf("    mov rax, rsp\n");
            printf("    and rax, 15\n");          // 下位4ビットだけ残す(16のあまりの部分)
            printf("    jnz .L.call.%03d\n", id); // 0じゃない場合ジャンプ
            printf("    mov rax, 0\n");
            printf("    call %s\n", node->funcname);
            printf("    jmp .L.end.%03d\n", id);
            printf(".L.call.%03d:\n", id);
            printf("    sub rsp, 8\n");
            printf("    mov rax, 0\n");           // 関数を呼ぶ前に常にALに0をセット
            printf("    call %s\n", node->funcname);
            printf("    add rsp, 8\n");
            printf(".L.end.%03d:\n", id);
            // callした返り値がraxにあるので、スタックトップにプッシュする
            // しないとpop raxによって上書きされてしまった
            printf("    push rax\n");
            return;

        case ND_FUNC_DEF:
            printf(".global %s\n", node->funcname);
            printf("%s:\n", node->funcname);
            // プロローグ
            printf("    push rbp\n");     // 現在のベースポインタの値をスタックにプッシュ
            printf("    mov rbp, rsp\n"); // スタックポインタをrbpにコピー
            // 引数以外の変数分のスタック領域を確保する
            if(locals[cur_func]){
                int offset = locals[cur_func]->offset;
                printf("    sub rsp, %d\n", offset); 
            }
            // 引数の値
            for(int i = 0; node->args[i]; i++){
                if(node->args[i]->size == 1){
                    printf("    mov [rbp-%d], %s\n", node->args[i]->offset, argreg1[i]);
                }
                else if(node->args[i]->size == 2){
                    printf("    mov [rbp-%d], %s\n", node->args[i]->offset, argreg2[i]); // 今は関係ない
                }
                else if(node->args[i]->size == 4){
                    printf("    mov [rbp-%d], %s\n", node->args[i]->offset, argreg4[i]);                   
                }
                else{
                    printf("    mov [rbp-%d], %s\n", node->args[i]->offset, argreg8[i]);
                }
                
            }
            
            gen(node->lhs);

            // エピローグ
            printf("    mov rsp, rbp\n"); // ベースポインタの値をrspに移動
            printf("    pop rbp\n");      // スタックから値をポップし，rbpに格納
            printf("    ret\n");          // スタックからリターンアドレスをポップする　呼び出し位置に戻る
            return;
        
        case ND_ADDR:
            gen_val(node->lhs);
            return;

        case ND_DEREF:
            gen(node->lhs);
            t = get_type(node);
            printf("    pop rax\n");
            if (t && t->ty == CHAR) {
                printf("    movsx rax, BYTE PTR [rax]\n");
            } 
            else if (t && t->ty == INT) {
                printf("    movsxd rax, DWORD PTR [rax]\n");
            } 
            else {
                printf("    mov rax, [rax]\n");
            }
            printf("    push rax\n");
            return;

        case ND_GVAR_DEF:
            printf("%s:\n", node->varname);
            printf("    .zero %d\n", node->size);
            return;
        }

    gen(node->lhs);     // 左の木を優先　// ノードの左辺の値をスタックにプッシュ
    gen(node->rhs);     // ノードの右辺の値をスタックにプッシュ

    printf("    pop rdi\n");    // rdiにスタックの値を格納
    printf("    pop rax\n");    // raxにスタックの値を格納

    // ノードが+, -, *, /, <, =<, =, !=によってアセンブリのコードを変える
    switch (node->kind){
        case ND_ADD:
            printf("    add rax, rdi\n");   // raxとrdiの和をraxに格納
            break;
            
        case ND_SUB:
            printf("    sub rax, rdi\n");   // raxとrdiの差をraxに格納
            break;

        case ND_MUL:
            printf("    imul rax, rdi\n");  // raxとrdiの積をraxに格納
            break;
            
        case ND_DIV:
            printf("    cqo\n");            // rdxとraxの値を128ビットとみなし，
            printf("    idiv rdi\n");       // rdiで割ったときの商をraxに格納
            break;

        case ND_EQ:
            printf("    cmp rax, rdi\n");   // raxとrdiレジスタの値を比べる
            printf("    sete al\n");        // レジスタの値が同じだった場合，alを1にする（seteは8ビットレジスタしか取れない）
            printf("    movzb rax, al\n");  // raxの上位56ビットを0クリアしながら，alの値8ビットを格納
            break;
            
        case ND_NE:
            printf("    cmp rax, rdi\n");   // raxとrdiレジスタの値を比べる
            printf("    setne al\n");       // レジスタの値が違った場合，alを1にする（setneは8ビットレジスタしか取れない）
            printf("    movzb rax, al\n");  // raxの上位56ビットを0クリアしながら，alの値8ビットを格納
            break;

        case ND_LT:
            printf("    cmp rax, rdi\n");   // raxとrdiレジスタの値を比べる
            printf("    setl al\n");        // raxレジスタの値がrdiレジスタの値より小さい場合，alを1にする（setlは8ビットレジスタしか取れない）
            printf("    movzb rax, al\n");  // raxの上位56ビットを0クリアしながら，alの値8ビットを格納
            break;

        case ND_LE:
            printf("    cmp rax, rdi\n");   // raxとrdiレジスタの値を比べる
            printf("    setle al\n");       // raxレジスタの値がrdiレジスタの値以下の場合，alを1にする（setleは8ビットレジスタしか取れない）
            printf("    movzb rax, al\n");  // raxの上位56ビットを0クリアしながら，alの値8ビットを格納
            break;
    }
    printf("    push rax\n");           // raxの値をスタックにプッシュ
}
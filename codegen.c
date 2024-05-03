#include "1cc.h"
void pop(char *pop_to);
void push(char *push_this);
void cmp(char *a, char *b);
void mov(char *to, char *from);
void mov1(char *to_with_brackets, char *from);
void mov2(char *to, char *from_with_brackets);
void mov3(char *to_with_brackets, char *from_with_brackets);

// ローカル変数リストからローカル変数を探し，そのポインタを返す
LVar *find_lvar_by_node(Node *node){
    LVar *a = locals;
    int offset = node->offset;
    while (a){
        if (a->offset == node->offset) return a;
        a = a->next;
    }
    return NULL;
}

void gen_lval(Node *node){
    // 変数 
    if(node->kind == ND_LVAR){
        mov("rax", "rbp");
        printf("    sub rax, %d\n", node->offset);
        push("rax");
        return;
    }
    // ポインタ変数
    else if(node->kind = ND_DEREF){
        gen(node->lhs);
        return;
    }
    error("代入の左辺値が正しくありません");
}

void gen(Node *node){
    int offset = 0;
    LVar *_lvar;
    int arg_index = 0;
    int num_args = 0;
    int current_label_index = label_index;
    label_index += 1;
    switch(node->kind){
        case ND_NUM:
            printf("    push %d\n", node->val); // ノードの値をスタックにプッシュ
            return;
            
        case ND_LVAR:
            gen_lval(node);        // ローカル変数のアドレスをスタックにプッシュ
            pop("rax");            // ローカル変数のアドレスをraxレジスタに格納
            mov2("rax", "rax");    // raxのアドレスにある値をraxに格納
            push("rax");           // スタックにプッシュ
            return;
        
        case ND_ASSIGN:
            gen_lval(node->lhs);   // 左辺のローカル変数のアドレスをスタックにプッシュ
            gen(node->rhs);        // 右辺の値をスタックにプッシュ
            pop("rdi");            // 右辺の値をrdiに格納
            pop("rax");            // 左辺のアドレスをraxに格納
            mov1("rax", "rdi");    // 左辺のアドレスに右辺の値を格納
            push("rdi");           // 代入された値をスタックにプッシュ
            return;
            
        case ND_RETURN:
            printf("    # return\n");   //アセンブリのコメント
            gen(node->lhs);             // return文の戻り値を表す式のアドレスをスタックにプッシュ
            pop("rax");                 // 戻り値を表す式のアドレスをraxレジスタに格納
            mov("rsp", "rbp");          // ベースポインタの値をスタックポインタの値に格納
            pop("rbp");                 // 前の関数のrbpの値を復元
            printf("    ret\n");        // 現在の関数から呼び出し元の関数に戻る
            return;

        case ND_IF:
            // if(node->condeition) node->lhsとなっている
            // 先にnode->condeitionのコードを作る
            gen(node->condition);
            // node->conditionのアドレスがスタックトップに残っているのでポップする
            pop("rax");
            // raxと0の値を比べる
            cmp("rax", "0");
            // raxの値が0ならifの条件は偽なので，node->lhsの処理は行わない
            printf("    je .Lend%03d\n", current_label_index);
            // ジャンプしなかった場合，node->lhsのコードを作る
            gen(node->lhs);
            printf(".Lend%03d:\n", current_label_index);
            return;

        case ND_IFELSE:
            // if(node->condtion) node->lhs else node->afetrthoughtとなっている
            // 先にnode->conditionのコードを作る
            gen(node->condition);
            // node->conditionのアドレスがスタックトップに残っているのでポップする
            pop("rax");
            // raxと0の値を比べる
            cmp("rax", "0");
            // raxの値が0ならifの条件は偽なので，node->lhsの処理は行わず，else文にジャンプ
            printf("    je .Lelse%03d\n", current_label_index);
            
            // ジャンプしなかった場合，ifの処理であるnode->lhsのコードを作る
            gen(node->lhs);
            // if文の処理をしたら，else文の処理であるnode->afterthoughtの処理は行わない
            printf("    jmp .Lend%03d\n", current_label_index);
            
            // else文の処理　node->afterthoughtのコードを作る
            printf(".Lelse%03d:\n", current_label_index);
            gen(node->afterthought);
            
            printf(".Lend%03d:\n", current_label_index);
            return;

        case ND_WHILE:
            // while(node->condition) node->lhsとなっている
            // while文の始まりのラベル
            printf(".Lbegin%03d:\n", current_label_index);
            // node->conditionのコードを作る
            gen(node->condition);
            // node->conditionのアドレスがスタックトップに残っているのでポップする
            pop("rax");
            // raxと0の値を比べる
            cmp("rax", "0");
            // raxの値が0ならwhileの条件は偽なので，node->lhsの処理は行わず，終わりラベルにジャンプ
            printf("    je .Lend%03d\n", current_label_index);
            
            // ジャンプしなかった場合，whileの処理であるnode->lhsのコードを作る
            gen(node->lhs);
            // ループするために開始ラベルにジャンプする
            printf("    jmp .Lbegin%03d\n", current_label_index);

            printf(".Lend%03d:\n", current_label_index);
            return;

        case ND_FOR:
            // for(node->initialize; node->condition; node->afterthought) node->lhsとなっている
            // 先にnode->initializeのコードを作る
            gen(node->initialize);
            // for文の始まりのラベル
            printf(".Lbegin%03d:\n", current_label_index);
            // node->conditionのコードを作る
            gen(node->condition);
            // node->conditionのアドレスがスタックトップに残っているのでポップする
            pop("rax");
            // raxと0の値を比べる
            cmp("rax", "0");
            // raxの値が0ならwhileの条件は偽なので，node->lhsの処理は行わず，終わりラベルにジャンプ
            printf("    je .Lend%03d\n", current_label_index);
            
            // node->lhsのコードを作る　処理を先に行う
            gen(node->lhs);
            // node->afterthougtのコードを作る
            gen(node->afterthought);
            
            // ループするために開始ラベルにジャンプする
            printf("    jmp .Lbegin%03d\n", current_label_index);

            printf(".Lend%03d:\n", current_label_index);
            return;

        case ND_BLOCK:
            // blockの配列の長さが空でない限り実行
            while(node->block->len){
                // 配列の最初のstmtをsub_nodeにする
                Node *sub_node = vec_get(node->block);
                // sub_nodeのコードを作る
                gen(sub_node);
                // sub_nodeのアドレスがスタックトップに残っているのでポップする
                pop("rax");
            }
            return;
        
        case ND_FUNCTION:
            num_args = node->arguments->len;
            arg_index = 0;
            // 奇数個の引数の場合、16バイト境界を保持するために調整
            if (num_args % 2 != 0) {
                printf("    sub rsp, 8\n");  // 16バイト境界の調整
            }
            while(node->arguments->len){
                gen(vec_pop(node->arguments));
            }
            arg_index = 0;
            while(arg_index++ < num_args){
                switch(arg_index){
                    case 1:
                        pop("rdi");
                        break;

                    case 2:
                        pop("rsi");
                        break;

                    case 3:
                        pop("rdx");
                        break;
                    
                    case 4:
                        pop("rcx");
                        break;
                    
                    case 5:
                        pop("r8");
                        break;

                    case 6:
                        pop("r9");
                        break;
                    
                    default:
                        error("6つより多い引数はサポートしていません");
                        break;
                }
            }
            
            // 呼び出し後にスタックを元に戻す
            if (num_args % 2 != 0) {
                printf("    add rsp, 8\n");
            }
            _lvar = find_lvar_by_node(node);
            printf("    call %.*s\n", _lvar->len, _lvar->name);
            push("rax");    // callした返り値がraxにあるので、スタックトップにプッシュする
            return;
        
        case ND_DEFINITION:
            _lvar = find_lvar_by_node(node);
            printf("    .global %.*s\n", _lvar->len, _lvar->name);
            printf("    .type    %.*s, @function\n", _lvar->len, _lvar->name);
            printf("%.*s:\n", _lvar->len, _lvar->name);
            printf(".LFB%d:\n", label_index);

            //プロローグ
            //変数128個分の領域を確保する
            printf("    #プロローグ\n");
            printf("    push rbp\n");       // 現在のベースポインタの値をスタックにプッシュ
            printf("    mov rbp, rsp\n");   // スタックポインタをrbpにコピー
            printf("    sub rsp, 1024\n");  // 128 * 8 = 1024

            num_args = node->arguments->len;
            arg_index = 0;
            while(arg_index++ < num_args){
                gen_lval(vec_get(node->arguments));
                pop("rax");
                switch(arg_index){
                    case 1:
                        mov1("rax","rdi");
                        break;
                    
                    case 2:
                        mov1("rax","rsi");
                        break;

                    case 3:
                        push("rdx");
                        break;
                    
                    case 4:
                        push("rcx");
                        break;
                    
                    case 5:
                        push("r8");
                        break;

                    case 6:
                        push("r9");
                        break;
                    
                    default:
                        error("6つより多い引数はサポートしていません");
                        break;
                }
                
            }
            while(node->block->len){
                Node *sub_node  = vec_get(node->block);
                gen(sub_node);
            }

            //エピローグ
            //最後の式の結果がRAXに残っているのでそれが返り値になる
            printf("    #エピローグ\n");
            printf("    mov rsp, rbp\n"); // ベースポインタの値をrspに移動
            printf("    pop rbp\n");      // スタックから値をポップし，rbpに格納
            printf("    ret\n");          // スタックからリターンアドレスをポップする　呼び出し位置に戻る
            return;

        case ND_ADDR:
            gen_lval(node->lhs);
            return;

        case ND_DEREF:
            gen(node->lhs);
            pop("rax");
            mov2("rax", "rax");
            push("rax");
            return;

    }

    gen(node->lhs);     // 左の木を優先 ノードの左辺の値をスタックにプッシュ
    gen(node->rhs);     // ノードの右辺の値をスタックにプッシュ

    pop("rdi");         // rdiにスタックの値を格納
    pop("rax");         // raxにスタックの値を格納

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
        cmp("rax", "rdi");              // raxとrdiレジスタの値を比べる
        printf("    sete al\n");        // レジスタの値が同じだった場合，alを1にする（seteは8ビットレジスタしか取れない）
        printf("    movzb rax, al\n");  // raxの上位56ビットを0クリアしながら，alの値8ビットを格納
        break;
    
    case ND_NE:
        cmp("rax", "rdi");              // raxとrdiレジスタの値を比べる
        printf("    setne al\n");       // レジスタの値が違った場合，alを1にする（setneは8ビットレジスタしか取れない）
        printf("    movzb rax, al\n");  // raxの上位56ビットを0クリアしながら，alの値8ビットを格納
        break;

    case ND_LT:
        cmp("rax", "rdi");              // raxとrdiレジスタの値を比べる
        printf("    setl al\n");        // raxレジスタの値がrdiレジスタの値より小さい場合，alを1にする（setlは8ビットレジスタしか取れない）
        printf("    movzb rax, al\n");  // raxの上位56ビットを0クリアしながら，alの値8ビットを格納
        break;

    case ND_LE:
        cmp("rax", "rdi");              // raxとrdiレジスタの値を比べる
        printf("    setle al\n");       // raxレジスタの値がrdiレジスタの値以下の場合，alを1にする（setleは8ビットレジスタしか取れない）
        printf("    movzb rax, al\n");  // raxの上位56ビットを0クリアしながら，alの値8ビットを格納
        break;
    }

    push("rax");                        // raxの値をスタックにプッシュ
}


// ここからアセンブリコードのを見やすくするために
// コメントを含んだコードを出力するための関数（デバック用）

void pop(char *pop_to){
    printf("    pop %s # スタックの先頭を%sにpop\n", pop_to, pop_to);
}

void push(char *push_this){
    printf("    push %s # スタックに%sをpush\n", push_this, push_this);
}
void cmp(char *a, char *b){
    printf("    cmp %s, %s # %sと%sを比較\n", a, b, a, b);
}

void mov(char *to, char *from){
    printf("    mov %s, %s # %sに%sからコピー\n", to, from, to, from);
}

void mov1(char *to_with_brackets, char *from){
    printf("    mov [%s], %s # %sが差すアドレスに%sの値をストア\n",
     to_with_brackets, from, to_with_brackets, from);
}

void mov2(char *to, char *from_with_brackets){
    printf("    mov %s, [%s] # %sに%sが差すアドレスからロード\n",
     to, from_with_brackets, to, from_with_brackets);
}

void mov3(char *to_with_brackets, char *from_with_brackets){
    printf("    mov [%s], [%s] # %sが差すアドレスに%sが差すアドレスからストア\n",
     to_with_brackets, from_with_brackets, to_with_brackets, from_with_brackets);
}
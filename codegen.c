#include "1cc.h"

void gen_lval(Node *node){
    if(node->kind != ND_LVAR){
        error("代入の左辺値が変数ではありません");
    }
    printf("    mov rax, rbp\n");               // ベースポインタをraxに移動
    printf("    sub rax, %d\n", node->offset);  // raxが変数のメモリアドレスを示す
    printf("    push rax\n");                   // ローカル変数のアドレスをスタックにプッシュ
    return;
}

void gen(Node *node){
    int current_label_index = label_index;
    label_index += 1;
    switch(node->kind){
        case ND_NUM:
            printf("    push %d\n", node->val); // ノードの値をスタックにプッシュ
            return;
            
        case ND_LVAR:
            gen_lval(node);                     // ローカル変数のアドレスをスタックにプッシュ
            printf("    pop rax\n");            // ローカル変数のアドレスをraxレジスタに格納
            printf("    mov rax, [rax]\n");     // raxのアドレスにある値をraxに格納
            printf("    push rax\n");           // スタックにプッシュ
            return;
        
        case ND_ASSIGN: // a = 1みたいなパターン
            gen_lval(node->lhs);                // 左辺のローカル変数のアドレスをスタックにプッシュ
            gen(node->rhs);                     // 右辺の値をスタックにプッシュ
            printf("    pop rdi\n");            // 右辺の値をrdiに格納
            printf("    pop rax\n");            // 左辺のアドレスをraxに格納
            printf("    mov [rax], rdi\n");     // 左辺のアドレスに右辺の値を格納
            printf("    push rdi\n");           // 代入された値をスタックにプッシュ
            return;
            
        case ND_RETURN: //return 4みたいなパターン
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
            printf("    je .Lend%03d\n", current_label_index);         // ゼロフラグが立っていれば処理終わりラベルにジャンプ
            // ジャンプしなかった場合，node->lhsのコードを作る
            gen(node->lhs);
            printf(".Lend%03d:\n", current_label_index);               // ジャンプ先（処理終わりラベル）
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
            printf("    je .Lelse%03d\n", current_label_index);        // ゼロフラグが立っていれば.Lelseラベルにジャンプ
            
            // ジャンプしなかった場合，ifの処理であるnode->lhsのコードを作る
            gen(node->lhs);
            // if文の処理をしたら，else文の処理であるnode->afterthoughtの処理は行わない
            printf("    jmp .Lend%03d\n", current_label_index);
            
            // else文の処理　node->afterthoughtのコードを作る
            printf(".Lelse%03d:\n", current_label_index);               // ジャンプ先（.Lelseラベル）
            gen(node->afterthought);
            
            printf(".Lend%03d:\n", current_label_index);               // ジャンプ先（処理終わりラベル）
            return;

        case ND_WHILE:
            // while(node->condition) node->lhsとなっている
            // while文の始まりのラベル
            printf(".Lbegin%03d:\n", current_label_index);
            // node->conditionのコードを作る
            gen(node->condition);
            // node->conditionのアドレスがスタックトップに残っているのでポップする
            printf("    pop rax\n");
            // raxと0の値を比べる
            printf("    cmp rax, 0\n");
            // raxの値が0ならwhileの条件は偽なので，node->lhsの処理は行わず，終わりラベルにジャンプ
            printf("    je .Lend%03d\n", current_label_index);        // ゼロフラグが立っていれば.Lendラベルにジャンプ
            
            // ジャンプしなかった場合，whileの処理であるnode->lhsのコードを作る
            gen(node->lhs);
            // ループするために開始ラベルにジャンプする
            printf("    jmp .Lbegin%03d\n", current_label_index);

            printf(".Lend%03d:\n", current_label_index);              // ジャンプ先（処理終わりラベル）
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
            printf("    pop rax\n");
            // raxと0の値を比べる
            printf("    cmp rax, 0\n");
            // raxの値が0ならwhileの条件は偽なので，node->lhsの処理は行わず，終わりラベルにジャンプ
            printf("    je .Lend%03d\n", current_label_index);        // ゼロフラグが立っていれば.Lendラベルにジャンプ
            
            // node->lhsのコードを作る　処理を先に行う
            gen(node->lhs);
            // node->afterthougtのコードを作る
            gen(node->afterthought);
            
            // ループするために開始ラベルにジャンプする
            printf("    jmp .Lbegin%03d\n", current_label_index);

            printf(".Lend%03d:\n", current_label_index);              // ジャンプ先（処理終わりラベル）
            return;

        case ND_BLOCK:
            // blockの配列の長さが空でない限り実行
            while(node->block->len){
                // 配列の最初のstmtをsub_nodeにする
                Node *sub_node = vec_get(node->block);
                // sub_nodeのコードを作る
                gen(sub_node);
                // sub_nodeのアドレスがスタックトップに残っているのでポップする
                printf("    pop rax\n");
            }
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
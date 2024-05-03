#include "1cc.h"

Node *expr();
Node *stmt();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
Node *assign();

//変数名を名前で探す．見つからなかった場合NULLを返す
LVar *find_lvar(Token *token){
    for(LVar *var = locals; var; var = var->next){
        if(var->len == token->len && !memcmp(var->name, token->str, token->len)){
            return var;
        }     
    }
    return NULL; 
}

//最後のローカル変数を位置を取得
LVar *getlastLVar(){
    LVar *tmp = locals; // ローカル変数の位置を取得
    while (tmp->next){  // 次のローカル変数がある限り進む
        tmp = tmp->next;    
    }
    return tmp;         // 最後のローカル変数の位置を返す
}

//左辺と右辺を受け取る2項演算子
Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

//数値を受け取る
Node *new_node_num(int val){
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM; 
    node->val = val;
    return node;
}

//　ここからパース

// パースの結果を保存しておく
void program(){
    int i = 0;
    while(!at_eof()){
        code[i++] = stmt();
    }
    code[i] = NULL; //配列の末尾をNULLにする
}

// 生成規則: stmt = expr ";" 
        // | "return" expr ";"
        // | "if" "(" expr ")" stmt ("else" stmt)?
        // | "while" "(" expr ")" stmt
        // | "for" "(" expr? ";" expr? ";" expr? ")" stmt
        // | "{" *stmt "}"
Node *stmt(){
    Node *node = node;
    
    // return node->lhs
    if(consume_kind(TK_RETURN)){       // returnトークンを使ったら
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;        // ノードの種類をreturnにする
        node->lhs = expr();            // ノード左辺をexprにする
        expect(";");
    }

    // if(node->condition) node->lhs
    else if(consume_kind(TK_IF)){
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->condition = expr();        // 条件文
        expect(")");
        node->lhs = stmt();              // ifの処理文
        // else node->afterthought
        if(consume_kind(TK_ELSE)){
            node->kind = ND_IFELSE;
            node->afterthought = stmt(); // elseの処理文
        }
    }

    // while(node->condition) node->lhs
    else if(consume_kind(TK_WHILE)){     // whileトークンを使ったら
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;           // ノードの種類をwhileにする
        expect("(");
        node->condition = expr();        // 条件文をexprにする
        expect(")");
        node->lhs = stmt();
    }

    // for(node->initialize; node->condition; node->afterthought) node->lhs
    else if(consume_kind(TK_FOR)){
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        expect("(");
        if(!consume(";")){
            node->initialize = expr();
            expect(";");
        }
        if(!consume(";")){
            node->condition = expr();
            expect(";");
        }
        if(!consume(")")){
            node->afterthought = expr();
            expect(")");
        }
        node->lhs = stmt();
    }

    else if(consume("{")){                  // "{"を消費したら
        node = calloc(1,sizeof(Node));
        node->kind = ND_BLOCK;              // ノードの種類をblockノード
        node->block = new_vec();            // ブロックを配列にする
        while(!consume("}")){               // "}"を消費するまでループ
            vec_push(node->block, stmt());  // stmtを配列の末尾に追加
        }
    }

    else{
        node = expr();
        if(node->kind != ND_DEFINITION){
            expect(";"); //関数定義でなければ最後の文字は;が来るはず
        }
    }
    return node;
}

// 生成規則: expr = assign
Node *expr(){
    return assign();
}

// 生成規則: assign = equality ("=" assign)?
Node *assign() {
    Node *node =  equality();
    if(consume("=")) {
        return new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

//生成規則: equality = relational ("==" relational | != relational)*
Node *equality(){
    Node *node = relational();

    for(;;){
        if(consume("==")){
            node = new_node(ND_EQ, node, relational());
        }
        else if(consume("!=")){
            node = new_node(ND_NE, node, relational());
        }
        else{
            return node;
        }
    }
}

//生成規則: relationl = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational(){
    Node *node = add();

    for(;;){
        if(consume("<")){
            node = new_node(ND_LT, node, add());
        }
        else if(consume("<=")){
            node = new_node(ND_LE, node, add());
        }
        else if(consume(">")){
            node = new_node(ND_LT, add(), node);    //順番を入れ替える
        }
        else if(consume(">=")){
            node = new_node(ND_LE, add(), node);
        }
        else{
            return node;
        }
    }
}

//生成規則: add = mul ("+" mul | "-" mul)*
Node *add(){
    Node *node = mul();
    for(;;){
        if(consume("+")){
            node = new_node(ND_ADD, node, mul());
        }
        else if(consume("-")){
            node = new_node(ND_SUB, node, mul());
        }
        else{
            return node;
        }
    }
}


//生成規則: mul = unary("*" unary | "/" unary)*
Node *mul(){
    Node *node = unary();

    for(;;){
        if(consume("*")){
            node = new_node(ND_MUL, node, unary());
        }
        else if(consume("/")){
            node = new_node(ND_DIV, node, unary());
        }
        else{
            return node;
        }
    }
}

// 生成規則: unary = "+"? primary
//                 | "-"? primary
//                 | "*" unary
//                 | "&" unary
Node *unary(){
    if(consume("+")){
        return primary(); // +xをxに変換
    }
    else if(consume("-")){
        // -xを0-xに変換
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    else if(consume("*")){
        // *X
        return new_node(ND_DEREF, unary(), NULL);
    }
    else if(consume("&")){
        // &x
        return new_node(ND_ADDR, unary(), NULL);
    }
    return primary();
}

//生成規則: primary = num | ident "(" ")"? | "(" expr ")"
Node *primary(){
    //次のトークンが"("なら"(" exrp ")
    if(consume("(")){
        Node *node = expr();
        expect(")"); //期待している記号が次に来るか確認
        return node;
    }

    // 識別子があるかチェック
    Type *type = check_type();
    Token *tok = consume_ident();
    if(tok) { // 次のトークンがIDENTの場合
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;        //ノードを変数として扱う
        
        LVar *lvar = find_lvar(tok); // 同じ名前の変数がないか確認
        if(lvar){ // あった場合
            node->offset = lvar->offset;   // 以前の変数のoffsetを使う
        }
        else if(type || check_next("{")){ // type:新しく定義する変数か関数 check_next:未定義の関数呼びだし
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;           // 変数の次をローカル変数にする
            lvar->name = tok->str;         // トークン文字列を新しい変数に設定
            lvar->len = tok->len;          // トークン文字列の長さを変数に設定
            if(locals){
                lvar->offset = locals->offset + 8;  // 新しい変数のオフセットを最後のローカル変数のオフセットの+8に設定
            }
            else{
                lvar->offset = 8;
            }
            lvar->type = type;             // 変数の型を決定
            node->offset = lvar->offset;   // ノードのオフセットを新しい変数のオフセットと同じにする
            locals = lvar;                 // ローカル変数に変数を入れる
        }
        else{
            error("未定義の変数です");
        }

        if(consume("(")){
            node->kind = ND_FUNCTION;
            node->arguments = new_vec();
            while(!consume(")")){
                vec_push(node->arguments, expr());
                consume(",");
            }
        }
        if(consume("{")){
            node->kind = ND_DEFINITION;
            node->block = new_vec();
            while(!consume("}")){
                vec_push(node->block, stmt());
            }
        }
        return node;
    }

    //そうでないときは数値
    return new_node_num(expect_number());
}
#include "1cc.h"
LVar *locals[100];
int cur_func=0;
Node *expr();
Node *func();
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
    for(LVar *var = locals[cur_func]; var; var = var->next){
        if(var->len == token->len && !memcmp(token->str, var->name, var->len)){
            return var;
        }     
    }
    return NULL; 
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
// program = func*
void program(){
    int i = 0;
    while(!at_eof()){
        code[i++] = func();
    }
    code[i] = NULL; //配列の末尾をNULLにする
}

// func = ident "(" ident ")"  stmt 
Node *func(){
    cur_func++;
    Node *node;
    Token *tok = consume_kind(TK_IDENT);
    if(tok == NULL){
        error("関数ではありません");
    }
    node = calloc(1, sizeof(Node));
    node->kind = ND_FUNC_DEF;
    node->funcname = calloc(100, sizeof(char));
    node->args = calloc(10, sizeof(Node*));
    memcpy(node->funcname, tok->str, tok->len);
    expect("(");
    for(int i=0; !consume(")"); i++){
        Token *tok = consume_kind(TK_IDENT);
        if(tok != NULL){
            // variable(tok)
            node->args[i] = variable(tok);
            memcpy(node->args[i], tok->str, tok->len);
        }
        if(consume(")")){
            break;
        }
        expect(",");
    }
    node->lhs = stmt();
    return node;
}

// 生成規則: stmt = expr ";" 
        // | "return" expr ";"
        // | "if" "(" expr ")" stmt ("else" stmt)?
        // | "while" "(" expr ")" stmt
        // | "for" "(" expr? ";" expr? ";" expr? ")" stmt
        // | "{" *stmt "}"
Node *stmt(){
    Node *node;
    // return node->lhs
    if(consume_kind(TK_RETURN)){       // returnトークンを使ったら
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;        // ノードの種類をreturnにする
        node->lhs = expr();            // ノード左辺をexprにする
        expect(";");
        return node;
    }

    // if(node->condition) node->lhs
    if(consume_kind(TK_IF)){
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
        return node;
    }

    // while(node->condition) node->lhs
    if(consume_kind(TK_WHILE)){     // whileトークンを使ったら
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;           // ノードの種類をwhileにする
        expect("(");
        node->condition = expr();        // 条件文をexprにする
        expect(")");
        node->lhs = stmt();
        return node;
    }

    // for(node->initialize; node->condition; node->afterthought) node->lhs
    if(consume_kind(TK_FOR)){
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
        return node;
    }

    // block
    if(consume("{")){                  // "{"を消費したら
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;              // ノードの種類をblockノード
        // 100行まで
        node->block = calloc(100, sizeof(Node));
        for(int i = 0; !consume("}"); i++){               // "}"を消費するまでループ
            node->block[i] = stmt();             // stmtを配列の末尾に追加
        }
        return node;
    }

    else{
        node = expr();
        expect(";"); //最後の文字は;が来るはず
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

// 生成規則: unary = ("+" | "-")? primary
Node *unary(){
    if(consume("+")){
        return unary(); // +xをxに変換
    }
    if(consume("-")){
        // -xを0-xに変換
        return new_node(ND_SUB, new_node_num(0), unary());
    }
    return primary();
}

//生成規則: primary = num 
//                   | ident ("(" expr* ")")? 
//                   | "(" expr ")"
Node *primary(){
    //次のトークンが"("なら"(" exrp ")
    if(consume("(")){
        Node *node = expr();
        expect(")"); //期待している記号が次に来るか確認
        return node;
    }

    Token *tok = consume_kind(TK_IDENT);
    if(tok){ // 次のトークンがIDENTの場合
        if(consume("(")){
            // 関数呼び出し
            Node *node = calloc(1, sizeof(Node));
            node->kind = ND_FUNC_CALL;
            node->funcname = calloc(100, sizeof(Node));
            memcpy(node->funcname, tok->str, tok->len);
            // 引数
            // とりあえず10個まで
            node->block = calloc(10, sizeof(Node));
            for(int i = 0; !consume(")"); i++){
                node->block[i] = expr();
                if (consume(")")){
                    break;
                }
                expect(",");      
            }
            return node;
        }
        // 関数呼び出しでない場合変数
        return variable(tok);
    }
    //そうでないときは数値
    return new_node_num(expect_number());
}


// 関数の引数の段階で変数を登録する
Node *variable(Token *tok){
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;        //ノードを変数として扱う
    
    LVar *lvar = find_lvar(tok); // 同じ名前の変数がないか確認
    if(lvar){ // あった場合
        node->offset = lvar->offset;   // 以前の変数のoffsetを使う
    }
    else{
        lvar = calloc(1, sizeof(LVar));
        lvar->next = locals[cur_func];
        lvar->name = tok->str;
        lvar->len = tok->len;
        if(locals[cur_func] == NULL){
            lvar->offset = 8;
        }
        else{
            lvar->offset = locals[cur_func]->offset + 8;
        }
        node->offset = lvar->offset;
        locals[cur_func] = lvar;
    }
    return node;
}
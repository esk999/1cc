#include "1cc.h"

//ここから再帰下降構文解析の実装

//
//パーサー
//
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
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

//最後のローカル変数を取得
LVar *getlastLocalsVar(){
    LVar *tmp = locals;
    while (tmp->next){
        tmp = tmp->next;
    }
    return tmp;
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

//
void program(){
    int i = 0;
    while(!at_eof()){
        code[i++] = stmt();
    }
    code[i] = NULL;
}

Node *stmt(){
    Node *node = expr();
    expect(";");
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

// 生成規則: unary = ("+" | "-")? unary | primary
Node *unary(){
    if(consume("+")){
        return primary(); // +xをxに変換
    }
    if(consume("-")){
        // -xを0-xに変換
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    return primary();
}

//生成規則: primary = num | "(" expr ")"
Node *primary(){
    //次のトークンが"("なら"(" exrp ")
    if(consume("(")){
        Node *node = expr();
        expect(")"); //期待している記号が次に来るか確認
        return node;
    }

    Token *tok = consume_ident();
    if(tok) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;
        
        LVar *lvar = find_lvar(tok);
        if(lvar){
            node->offset = lvar->offset;
        }
        else{
            lvar = calloc(1, sizeof(LVar));
            lvar->name = tok->str;
            lvar->len = tok->len;
            LVar *prevVar = getlastLocalsVar();
            lvar->offset = prevVar->offset + 8;
            prevVar->next = lvar;
            node->offset = lvar->offset;
        }
        return node;
    }

    //そうでないときは数値
    return new_node_num(expect_number());
}
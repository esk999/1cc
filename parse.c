#include "1cc.h"
LVar *locals[100];
LVar *globals[100];
int cur_func=0;
StringToken *strings;
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
LVar *find_variable(Token *token){
    for(LVar *var = locals[cur_func]; var; var = var->next){
        if(var->len == token->len && !memcmp(token->str, var->name, var->len)){
            var->kind = LOCAL;
            return var;
        }     
    }
    for(LVar *var = globals[0]; var; var = var->next){
        if(var->len == token->len && !memcmp(token->str, var->name, var->len)){
            var->kind = GLOBAL;
            return var;
        }     
    }
    return NULL; 
}

// 型名の前半を読む関数
Define *read_define(){
    Token *typetoken = consume_kind(TK_TYPE);
    if(!typetoken){
        return NULL;
    }
    Type *type = calloc(1, sizeof(Type));
    int isChar = memcmp("char", typetoken->str, typetoken->len) == 0;
    type->ty = isChar ? CHAR : INT;
    type->ptr_to = NULL;
    while(consume("*")){
        Type *t = calloc(1, sizeof(Type));
        t->ty = PTR;
        t->ptr_to = type;
        type = t;
    }
    Token *tok = consume_kind(TK_IDENT);
    if(tok == NULL){
        error("関数または変数ではありません");
    }
    Define *def = calloc(1, sizeof(Define));
    def->type = type;
    def->ident = tok;
    return def;
}


// ノードの型を取得する
Type *get_type(Node *node){
    if(node == NULL){
        return NULL;    //sizeof(1)のとき
    }

    if(node->type){
        return node->type;
    }
    Type *t = get_type(node->lhs);
    if(!t){
        t = get_type(node->rhs);
    }
    if(t && node->kind == ND_DEREF){
        t = t->ptr_to;
        if(t == NULL){
            error("無効な参照です");
        }
        return t;
    }
    return t;
}

Node *new_node_one(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
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
    Node *node = new_node_one(ND_NUM);
    node->val = val;
    return node;
}

// stringノードを生成
Node *new_node_string(StringToken *s){
    Node *node = new_node_one(ND_STRING);
    node->string = s;
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

// func = "int" ident "(" ("int" ident ("," "int" ident)* ) ? ")" stmt 
Node *func(){
    Node *node;
    Define *def = read_define();
    if(consume("(")){
        cur_func++;
        node = calloc(1, sizeof(Node));
        node->kind = ND_FUNC_DEF;
        node->funcname = calloc(100, sizeof(char));
        node->args = calloc(10, sizeof(Node*));
        memcpy(node->funcname, def->ident->str, def->ident->len);
        for(int i = 0; !consume(")"); i++){
            node->args[i] = define_variable(read_define(), locals);
            if(consume(")")){
                break;
            }
            expect(",");
        }
        node->lhs = stmt();
        return node;
    }
    else{
        // 変数定義
        node = define_variable(def, globals); //グローバル変数
        node->kind = ND_GVAR_DEF;
        expect(";");
        return node;
    }
}

// 生成規則: stmt = expr ";" 
        // | "return" expr ";"
        // | "if" "(" expr ")" stmt ("else" stmt)?
        // | "while" "(" expr ")" stmt
        // | "for" "(" expr? ";" expr? ";" expr? ")" stmt
        // | "{" *stmt "}"
        // | int "*"* "ident" ";"
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
    // int
    Define *def = read_define();
    if(def){
        node = define_variable(def, locals);
        expect(";");
        return node;
    }
    node = expr();
    expect(";"); //最後の文字は;が来るはず
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
            Node *r = mul();
            if(node->type && node->type->ty != INT){           // ARRAY, PTR, CHARのとき実行
                int n = node->type->ptr_to->ty == INT ? 4 
                      : node->type->ptr_to->ty == CHAR ? 1 : 8; 
                r = new_node(ND_MUL, r, new_node_num(n));
            }
            node = new_node(ND_ADD, node, r);
        }
        else if(consume("-")){
            Node *r = mul();
            if(node->type && node->type->ty != INT){           // ARRAY, PTR, CHARのとき実行
                int n = node->type->ptr_to->ty == INT ? 4
                      : node->type->ptr_to->ty == CHAR ? 1 : 8; 
                r = new_node(ND_MUL, r, new_node_num(n));
            }
            node = new_node(ND_SUB, node, r);
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

// 生成規則: unary = "+"? unary
//                 | "-"? unary
//                 | "*" unary
//                 | "&" unary 
//                 | "sizeof" unary
Node *unary(){
    if(consume("+")){
        return unary(); // +xをxに変換
    }
    if(consume("-")){
        // -xを0-xに変換
        return new_node(ND_SUB, new_node_num(0), unary());
    }
    if(consume("*")){
        return new_node(ND_DEREF, unary(), NULL);
    }
    if(consume("&")){
        return new_node(ND_ADDR, unary(), NULL);
    }
    if(consume_kind(TK_SIZEOF)){
        Node *n = unary();
        Type *t = get_type(n);
        int size = t && t->ty == PTR ? 8 
                 : t && t->ty == CHAR ? 1 : 4;
        return new_node_num(size);
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

    // string
    if(tok = consume_kind(TK_STRING)){
        StringToken *s = calloc(1, sizeof(StringToken));
        s->value = calloc(100, sizeof(char));
        memcpy(s->value, tok->str, tok->len);
        if(strings){
            s->index = strings->index + 1;
        }
        else{
             s->index = 0;
        }
        s->next = strings;
        strings = s;
        return new_node_string(s);
    }

    //そうでないときは数値
    return new_node_num(expect_number());
}

// 関数の引数の段階で変数を登録する
Node *variable(Token *tok){
    Node *node = calloc(1, sizeof(Node));    
    LVar *lvar = find_variable(tok); // 同じ名前の変数がないか確認
    node->varname = calloc(100, sizeof(char));
    memcpy(node->varname, tok->str, tok->len);
    if(lvar == NULL){
        error("%sは未定義の変数です", node->varname);
    }
    node->kind = lvar->kind == LOCAL ? ND_LVAR : ND_GVAR;
    node->offset = lvar->offset;   // 以前の変数のoffsetを使う
    node->type = lvar->type;

    // a[2]を*(a + 2)と解釈する
    // 二重配列は未実装
    while(consume("[")){ 
        Node *add = calloc(1, sizeof(Node));
        add->kind = ND_ADD;
        add->lhs = node;                // 配列名を表す式
        if (node->type && node->type->ty != INT) {
            int n = node->type->ptr_to->ty == INT ? 4
                : node->type->ptr_to->ty == CHAR ? 1 : 8;
            add->rhs = new_node(ND_MUL, expr(), new_node_num(n));
        }
        node = calloc(1, sizeof(Node)); // メモリを確保して初期化
        node->kind = ND_DEREF;
        node->lhs = add;                // ポインタ + インデックスの値を取得
        expect("]");
    }

    return node;
}

Node *define_variable(Define *def, LVar **varlist){
    if(def == NULL){
        error("無効な定義です");
    }
    Type *type = def->type;
    int size = type->ty == PTR ? 8 : type->ty == CHAR ? 1 : 4;
    // 配列か確認する
    while(consume("[")){
        Type *t= calloc(1, sizeof(Type));
        t->ty = ARRAY;
        t->ptr_to = type;
        t->array_size = expect_number();
        type = t;
        size *= t->array_size;
        expect("]");
    }

    Node *node = calloc(1, sizeof(Node));
    node->varname = calloc(100, sizeof(char));
    memcpy(node->varname, def->ident->str, def->ident->len);
    node->size = size;
    LVar *lvar = find_variable(def->ident); // 同じ名前の変数がないか確認
    if(lvar != NULL){
        error("%sは未定義の変数です", node->varname);
    }
    // 要修正
    node->kind = locals == varlist ? ND_LVAR : ND_GVAR;        //変数の子類によってノードを切り替える
    lvar = calloc(1, sizeof(LVar));
    lvar->next = varlist[cur_func];
    lvar->name = def->ident->str;
    lvar->len = def->ident->len;
    if(varlist[cur_func] == NULL){
        lvar->offset = size;
    }
    else{
        lvar->offset = varlist[cur_func]->offset + size;
    }
    lvar->type = type;
    node->offset = lvar->offset;
    node->type = lvar->type;
    varlist[cur_func] = lvar;
    return node;
}
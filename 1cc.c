#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//
//トークナイザ
//

//トークンの種類
typedef enum{
    TK_RESERVED,    //記号
    TK_NUM,         //整数トークン
    TK_EOF,         //入力終わりを表すトークン
} TokenKind;

typedef struct Token Token;

//トークン型
struct Token{
    TokenKind kind; //トークンの型
    Token *next;    //次の入力トークン
    int val;        //kindがTK_NUMの場合，その数値
    char *str;      //トークン文字列
    int len;        //トークンの長さ
};

//現在注目しているトークン
Token *token;

//エラーを報告するための関数
//printfと同じ引数をとる
void error(char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

//入力プログラム
char *user_input;

//エラー個所を報告
void error_at(char *loc, char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); //pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

/*
次のトークンが期待している記号の時には，トークン1つを読み進めて
真を返す．それ以外の場合には偽を返す．
*/
bool consume(char *op){
    /*
        複数の文字をトークナイズする場合，長いトークンから先にトークナイズする
        文字列が>から始まる場合，>=が>と=に誤ってトークナイズされてしまうから．
    */
    if(token->kind != TK_RESERVED || 
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len)){
        return false;
    }
    token = token -> next;
    return true;
}

/*
次のトークンが期待している記号の時には，トークンを1つ読み進める．
それ以外の場合はエラーを報告する．
*/
void expect(char *op){
    if(token->kind != TK_RESERVED || 
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len)){
        error_at(token->str, "expected \"%s\"", op);
    }
    token = token->next;
}

/*
次のトークンが数値の場合，トークンを1つ読み進めてその数値を返す．
それ以外の場合はエラーを報告する．
*/
int expect_number(){
    if(token->kind != TK_NUM){
        error_at(token->str, "expected a number");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof(){
    return token->kind == TK_EOF;
}

//新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str, int len){
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswitch(char *p, char *q){
    return memcmp(p, q, strlen(q)) == 0;
}

//入力文字列pをトークナイズしてそれを返す
Token *tokenize(){
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while(*p){
        //空白文字をスキップ
        if(isspace(*p)){
            p++;
            continue;
        }

        //2文字の比較演算子のときのトークナイズの長さを2にする
        if(startswitch(p, "==") || startswitch(p, "!=") ||
            startswitch(p, "<=") || startswitch(p, ">=")){
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2; //2個進める
            continue;;
        }
        
        //1文字の比較演算子と括弧と計算記号
        if(strchr("+-*/()<>", *p)){
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        //数値の場合
        if(isdigit(*p)){
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error_at(p, "invalid token");
    }
    
    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

//ここから再帰下降構文解析の実装

//
//パーサー
//

//抽象構文木のノードの種類
typedef enum{
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NUM, // 整数
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
} NodeKind;

typedef struct Node Node;

//抽象構文木のノードの型
struct Node{
    NodeKind kind; //ノードの型
    Node *lhs;     //左辺
    Node *rhs;     //右辺
    int val;       //kindがND_NUMの場合のみ扱う
};

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

//先に関数宣言
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();


// 生成規則: expr = equality
Node *expr(){
    return equality();
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
        return new_node(ND_SUB, new_node_num(0), unary());
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

    //そうでないときは数値
    return new_node_num(expect_number());
}

//
//アセンブリのコード生成
//

void gen(Node *node){
    if(node->kind == ND_NUM){
        printf("    push %d\n", node->val);
        return;
    }

    gen(node->lhs);     //左の木を優先
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    //ノードが+-*/によってアセンブリのコードを変える
    switch (node->kind){
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;

    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;

    case ND_EQ:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    
    case ND_NE:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;

    case ND_LT:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;

    case ND_LE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    }

    printf("    push rax\n");
}

int main(int argc, char **argv){
    if(argc != 2){
        error("%s: invalid number of arguments", argv[0]);
        return 1;
    }
    //トークナイズしてパースする
    user_input = argv[1];
    token = tokenize(user_input);
    Node *node = expr();
    

    //アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    //抽象構文木を下りながらコードを生成
    gen(node);

    //スタックトップに式全体の値が残っている
    //それをRAXにロードして関数からの帰り値とする
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}
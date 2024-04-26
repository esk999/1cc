#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//トークンの種類
typedef enum{
    TK_RESERVED,    //記号
    TK_IDENT,     // 識別子
    TK_NUM,         //整数トークン
    TK_EOF,         //入力終わりを表すトークン
} TokenKind;

typedef struct Token Token;
//トークン型
struct Token{
    TokenKind kind; //トークンの型
    struct Token *next;    //次の入力トークン
    int val;        //kindがTK_NUMの場合，その数値
    char *str;      //トークン文字列
    int len;        //トークンの長さ
};

typedef struct LVar LVar;

struct LVar{
    struct LVar *next; //次の変数かNULL
    char *name; //変数の名前
    int len;    //名前の長さ
    int offset; //RBPからのオフセット
};

//ローカル変数  
LVar *locals;

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
    ND_ASSIGN, // =
    ND_LVAR,  // ローカル変数
} NodeKind;

typedef struct Node Node;
//抽象構文木のノードの型
struct Node{
    NodeKind kind; //ノードの型
    Node *lhs;     //左辺
    Node *rhs;     //右辺
    int val;       //kindがND_NUMの場合のみ扱う
    int offset;    //kindがND_LVARの場合のみ扱う
};

//入力プログラム
char *user_input;

//現在注目しているトークン
Token *token;

//パーサ
Node *expr();
Node *code[100];
void program();
LVar *find_lvar(Token *token);

//トークナイザ
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str);
bool startswitch(char *p, char *q);
void tokenize();
Token *consume_ident();

//コード生成    codegen.c
void gen(Node *node);
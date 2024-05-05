#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

// // コンテナ
// // 動的なサイズ変更が可能な配列を実装
// typedef struct Vector Vector;

// struct Vector{
//     void **data;    // 任意の型のデータを保持する
//     int capacity;   // 保持できる最大の要素数
//     int len;        // 格納されている要素数
// };

// // キーと値のペアを格納するための単純な連想配列（マップ）を実装
// typedef struct Map Map;

// struct Map{
//     Vector *keys;   // キー
//     Vector *vals;   // 値
// };

// container.c
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
// // Vector *new_vec();
// void vec_push(Vector *v, void *element);
// void vec_pushi(Vector *v, int val);
// void *vec_pop(Vector *v);
// void *vec_get(Vector *v);
// void *vec_last(Vector *v);
// bool vec_contains(Vector *v, void *element);
// bool vec_union1(Vector *v, void *element);

// Map *new_map(void);
// void map_put(Map *map, char *key, void *val);
// void map_puti(Map *map, char *key, int val);
// void *map_get(Map *map, char *key);
// int map_geti(Map *map, char *key, int default_);

//トークンの種類
typedef enum{
    TK_RESERVED,    //記号
    TK_IDENT,       //識別子
    TK_NUM,         //整数トークン
    TK_EOF,         //入力終わりを表すトークン
    TK_RETURN,      //returnトークン
    TK_IF,          //ifトークン
    TK_ELSE,        //elseトークン
    TK_WHILE,       //whileトークン
    TK_FOR,         //forトークン
    TK_TYPE,        // int
} TokenKind;

typedef struct Token Token;
//トークン型
struct Token{
    TokenKind kind;        //トークンの型
    struct Token *next;    //次の入力トークン
    int val;               //kindがTK_NUMの場合，その数値
    char *str;             //トークン文字列
    int len;               //トークンの長さ
};

typedef struct Type Type;

struct Type{
    enum {INT, PTR} ty;
    struct Type *ptr_to;
};

typedef struct LVar LVar;

struct LVar{
    struct LVar *next; //次の変数かNULL
    char *name;        //変数の名前
    int len;           //名前の長さ
    int offset;        //RBPからのオフセット
    Type *type;
};

//抽象構文木のノードの種類
typedef enum{
    ND_ADD,     // +
    ND_SUB,     // -
    ND_MUL,     // *
    ND_DIV,     // /
    ND_NUM,     // 整数
    ND_EQ,      // ==
    ND_NE,      // !=
    ND_LT,      // <
    ND_LE,      // <=
    ND_ASSIGN,  // =
    ND_LVAR,    // ローカル変数
    ND_RETURN,  // returnノード
    ND_IF,      // ifノード
    ND_IFELSE,  // if elseノード
    ND_WHILE,   // whileノード
    ND_FOR,     // forノード
    ND_BLOCK,   // blockノード
    ND_FUNC_CALL,    // 関数
    ND_FUNC_DEF, // 関数定義
    ND_ADDR,    // &
    ND_DEREF,   // *
} NodeKind;

typedef struct Node Node;
//抽象構文木のノードの型
struct Node{
    NodeKind kind;      //ノードの型
    Node *lhs;          //左辺
    Node *rhs;          //右辺
    int val;            //kindがND_NUMの場合のみ扱う
    int offset;         //kindがND_LVARの場合のみ扱う
    Node *condition;    // while，for，ifの条件文に使う
    Node *afterthought; // if-else，forの処理文に使う
    Node *initialize;   // forのみ条件文に使う
    Node **block;       // blockに使う
    // Vector *block;      // blockに使う
    // Vector *arguments;  // kindがND_FUNCの場合のみ扱う
    char *funcname;     // ND_FUNCのみ使う
    Node **args;        // ND_FUNC_DEFのみ使う
    Type *type;         // ND_LVARのみ使う
};
// ローカル変数  
extern LVar *locals[];
extern int cur_func;
// 入力プログラム
char *user_input;

// 現在注目しているトークン
Token *token;

// アセンブリの中のラベルインデックス
int label_index;

//パーサ parse.c
Node *code[100];
void program();
Node *variable(Token *tok);
Node *define_variable();

//トークナイザ tokenize.c
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str);
void tokenize(char *p);
Token *consume_kind(TokenKind kind);

//コード生成    codegen.c
void gen(Node *node);
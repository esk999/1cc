#include "1cc.h"

// 次のトークンが期待している記号の時には，トークン1つを読み進めて
// 真を返す．それ以外の場合には偽を返す．

bool consume(char *op){
    
    // 複数の文字をトークナイズする場合，長いトークンから先にトークナイズする
    // 文字列が>から始まる場合，>=が>と=に誤ってトークナイズされてしまうから．
   
    if(token->kind != TK_RESERVED || 
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len)){
        return false;
    }
    token = token -> next;
    return true;
}


// 次のトークンが期待している記号の時には，トークンを1つ読み進める．
// それ以外の場合はエラーを報告する．

void expect(char *op){
    if(token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len)){ 
        error_at(token->str, "\"%s\"ではありません", op);
    }
    token = token->next;
}


// 次のトークンが数値の場合，トークンを1つ読み進めてその数値を返す．
// それ以外の場合はエラーを報告する．

int expect_number(){
    if(token->kind != TK_NUM){
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

// 入力終わりのトークンか判定
bool at_eof(){
    if(token->kind == TK_EOF){
        return true;
    }
    return false;
}

// 与えられた文字が英数字かアンダースコアかどうかを判定
int is_alnum(char c){
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

Token *consume_kind(TokenKind kind) {
    if (token->kind != kind) {
        return NULL;
    }
    Token* tok = token;
    token = token->next;
    return tok;
}

//新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str){
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;   // トークンの種類を決定
    tok->str = str;     // トークンの文字列を決定
    cur->next = tok;    // 前のトークンに新しいトークンを繋げる
    return tok;         // 新しいトークンを返す
}

//長さが一致しているかを判定
bool startswitch(char *p, char *q){
    //　p, qの長さが一致していたらtrue，それ以外はfalse
    return memcmp(p, q, strlen(q)) == 0;
}

typedef struct ReservedWord ReservedWord;
struct ReservedWord {
    char *word;
    TokenKind kind;
};

ReservedWord reservedWords[] = { 
    {"return", TK_RETURN},
    {"if", TK_IF},
    {"else", TK_ELSE},
    {"while", TK_WHILE},
    {"for", TK_FOR},
    {"int", TK_TYPE},
    {"char", TK_TYPE},
    {"sizeof", TK_SIZEOF},
    {"", TK_EOF},
};

// 入力文字列pをトークナイズしてそれを返す
// 連結リストを作成する
void tokenize(char *p){
    Token head;         // ダミーのheadと要素
    head.next = NULL;   // 現在のリストがheadだけということを示す
    Token *cur = &head; // 連結リストの現在の位置を示すのに使用する

    while(*p){
        //空白文字をスキップ
        if(isspace(*p)){
            p++;
            continue;
        }

        // 行コメントをスキップ
        if (strncmp(p, "//", 2) == 0) {
            p += 2;
            while (*p != '\n'){
                p++;
            }       
            continue;
        }

        // ブロックコメントをスキップ
        if (strncmp(p, "/*", 2) == 0) {
        char *q = strstr(p + 2, "*/");
        if (!q){
            error_at(p, "コメントが閉じられていません");
        }
        p = q + 2;
        continue;
        }


        //2文字の比較演算子のときのトークナイズの長さを2にする
        if(startswitch(p, "==") || startswitch(p, "!=") ||
            startswitch(p, "<=") || startswitch(p, ">=")){
            cur = new_token(TK_RESERVED, cur, p);
            cur->len = 2;
            p += 2; //2個進める
            continue;;
        }
        
        //1文字の比較演算子と括弧と計算記号
        if(strchr("+-*/()<>=;{},&[]", *p)){
            cur = new_token(TK_RESERVED, cur, p++); //1字進める
            cur->len = 1; // curの長さを1
            continue;
        }

        //数値の場合
        if(isdigit(*p)){
            cur = new_token(TK_NUM, cur, p);
            // pを10進数でlong int型の数値に変換
            // 第2引数でpが数値の直後の文字の位置をさすように更新
            cur->val = strtol(p, &p, 10);
            continue;
        }

        if('"' == *p){
            p++; // "の次の文字から考える
            char *c = p;
            while('"' != *c){
                c++;
            }
            int len = c - p;
            cur = new_token(TK_STRING, cur, p);
            cur->len = len;
            p = c;
            p++; // "の次の文字に進む
            continue;
        }

        //予約語
        bool found = false;
        for (int i = 0; reservedWords[i].kind != TK_EOF; i++) {
            char *w = reservedWords[i].word;
            int len = strlen(w);
            TokenKind kind = reservedWords[i].kind;
            if (startswitch(p, w) && !is_alnum(p[len])) {
                cur = new_token(kind, cur, p);
                cur->len = len;
                p += len;
                found = true;
                break;
            }
        }
        if (found) {
            continue;
        }

        // 変数
        if(is_alnum(*p)) {
            int varlen = 0;
            while (is_alnum(p[varlen])){ // 変数名の長さを取得
                varlen += 1;
            }
            cur = new_token(TK_IDENT, cur, p);
            cur->len = varlen;  //ノードの長さをvarlenにする
            p = p + varlen;     //varlenの分だけ文字を進める
            continue;
        }
        // それ以外の場合
        error_at(p, "トークナイズできません");
    }

    // トークンの終わりを示すトークンを作成
    new_token(TK_EOF, cur, p);
    token = head.next;
    return;
}
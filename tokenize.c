#include "1cc.h"

//エラーを報告するための関数
//printfと同じ引数をとる
void error(char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}



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
        error_at(token->str, "\"%s\"ではありません", op);
    }
    token = token->next;
}

/*
次のトークンが数値の場合，トークンを1つ読み進めてその数値を返す．
それ以外の場合はエラーを報告する．
*/
int expect_number(){
    if(token->kind != TK_NUM){
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof(){
    if(token->kind == TK_EOF){
        return true;
    }
    return false;
}

//変数名の長さを取得する関数
int getLVarlength(char *p){
    char *target = p;
    int length = 0; //長さのカウンタ

    while('a' <= *target && *target <= 'z'){
            length++;  //カウントを1増やす
            target++; //次の文字に進める
    }
    return length;
}

int is_alnum(char c){
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

Token *consume_ident(){
    if(token->kind == TK_IDENT) {
        Token *identToken = token;
        token = token->next;
        return identToken;
    }

    return NULL;
}

Token *consume_return(){
    if(token->kind == TK_RETURN) {
        Token *returnToken = token;
        token = token->next;
        return returnToken;
    }

    return NULL;
}

//新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str){
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

//長さが一致しているかを判定
bool startswitch(char *p, char *q){
    return memcmp(p, q, strlen(q)) == 0;
}

//入力文字列pをトークナイズしてそれを返す
void tokenize(char *p){
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
            cur = new_token(TK_RESERVED, cur, p);
            cur->len = 2;
            p += 2; //2個進める
            continue;;
        }
        
        //1文字の比較演算子と括弧と計算記号
        if(strchr("+-*/()<>=;", *p)){
            cur = new_token(TK_RESERVED, cur, p++);
            cur -> len = 1;
            continue;
        }

        //数値の場合
        if(isdigit(*p)){
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        //予約語
        //変数より先に判定するようにした
        //return    
        if(strncmp(p, "return", 6) == 0 && !is_alnum(p[6])){
            cur = new_token(TK_RESERVED, cur, p);
            cur->len = 6; //returnの文字数にする
            p = p + 6; //returnの文字数分進める
            continue;
        }

        //変数
        //aからzで始まる変数
        if('a' <= *p && *p <='z') {
            int length = getLVarlength(p);
            cur = new_token(TK_IDENT, cur, p);
            cur->len = length; //ノードの長さをlengthにする
            p = p + length; //lengthの分だけ文字を進める
            continue;
        }
        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p);
    token = head.next;
    return;
}
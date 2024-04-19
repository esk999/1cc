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

Token *consume_ident(){
    if(token->kind == TK_IDENT) {
        Token *identToken = token;
        token = token->next;
        return identToken;
    }

    // ひとまず0を返す 当初returnを省略してたら、謎のアドレスが返されていたので。
    return 0;
}

//新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str){
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
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
            cur = new_token(TK_RESERVED, cur, p);
            cur -> len = 2;
            p += 2; //2個進める
            continue;;
        }
        
        //1文字の比較演算子と括弧と計算記号
        if(strchr("+-*/()<>", *p)){
            cur = new_token(TK_RESERVED, cur, p++);
            cur -> len = 1;
            continue;
        }

        //数値の場合
        if(isdigit(*p)){
            cur = new_token(TK_NUM, cur, p);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        //変数
        if('a' <= *p && *p <='z') {
            cur = new_token(TK_IDENT, cur, p++);
            cur->len = 1;
            continue;
        }

        error_at(p, "invalid token");
    }
    
    new_token(TK_EOF, cur, p);
    return head.next;
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
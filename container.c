#include "1cc.h"

//エラー個所を報告
void error_at(char *loc, char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    char *p = user_input;
    int buffered = 0;
    char buffer[1024];
    int offset = 0;
    int pos = loc - user_input - 1;
    int line_index = 1;

    while(*p){
        if(*p != '\n'){                 // 改行を検出すると，それまでの行をバッファに保存
            buffer[buffered++] = *p;
        }
        else{                           // エラーがその行にあるか確認
            buffer[buffered++] = '\0';  
            if(offset < pos && pos <= offset + buffered){
                char msg_header[1024];
                snprintf(msg_header, sizeof(msg_header), "line:%d", line_index);
                fprintf(stderr, "%s%s\n", msg_header, buffer);
                fprintf(stderr, "%*s", (int)strlen(msg_header), "");
                fprintf(stderr, "^ ");
                vfprintf(stderr, fmt, ap);
                fprintf(stderr, "\n");
                exit(1);
            }
            offset += buffered;
            buffered = 0;
            line_index +=1 ;
        }
        p += 1;
    }
    char msg_header[1024];
    snprintf(msg_header, sizeof(msg_header), "line:%d", line_index);
    fprintf(stderr, "%s%s\n", msg_header, buffer);
    fprintf(stderr, "%*s", (int)strlen(msg_header) + pos + 1, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

//エラーを報告するための関数
//printfと同じ引数をとる
void error(char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// // 新しいVector配列を生成
// Vector *new_vec(){
//     Vector *v = malloc(sizeof(Vector));     // メモリ割り当て
//     v->data = malloc(sizeof(void *) * 16);  // dataポインタ配列のために初期容量として16の要素分のメモリを割り当て
//     v->capacity = 16;                       // 容量を16に設定
//     v->len = 0;                             // 新しいVectorは空
//     return v;                               // 初期化されたVectorのポインタを返す
// }

// // Vector配列の末尾に要素を追加
// void vec_push(Vector *v, void *element){    // *v : 追加する配列，*element :　追加する要素(任意の型)
//     if(v->len == v->capacity){              // Vectorの容量がいっぱいの場合
//         v->capacity *= 2;                   // 容量を2倍にし，
//         v->data = realloc(v->data, sizeof(void *) * v->capacity);  // data配列のサイズを新しい容量に合わせて拡張する
//     }
//     v->data[v->len++] = element;            // 配列の末尾に新しい要素を追加
// }

// // Vector配列の末尾に整数型の要素を追加
// void vec_pushi(Vector *v, int val){
//     vec_push(v, (void *)(intptr_t) val);    // intptr_tは整数型をポインタ型に変換
// }

// // Vector配列の末尾の要素を削除する
// void *vec_pop(Vector *v){
//     assert(v->len);                         // Vectorが空でないことを確認
//     return v->data[--v->len];               // lenの長さを1つ減らす
// }

// void *vec_get(Vector *v){
//     assert(v->len);                         // Vectorが空でないことを確認
//     void *ret = v->data[0];                 // Vectorの最初の要素を戻り値にする
//     Vector *_v = new_vec();                 // 元のベクトルから要素が削除された後の要素を格納するために使用
//     for(int i = 1; i < v->len; i++){        
//         vec_push(_v, v->data[i]);           // 最初の要素が削除された新しいリストを作成
//     }
//     //最初の要素が削除された状態に更新
//     v->data = _v->data;
//     v->capacity = _v->capacity;
//     v->len = _v->len;
//     return ret;
// }

// // Vector配列の末尾の要素を参照する
// void *vec_last(Vector *v){
//     assert(v->len);                         // Vectorが空でないことを確認
//     return v->data[v->len - 1];             // 配列の末尾の要素を返り値にする
// }

// // Vector配列に要素が存在するか確認
// bool vec_contains(Vector *v, void *element){
//     for(int i = 0; i < v-> len; i++){          // 配列内の要素すべてを確認
//         if(v->data[i] == element) return true; // 一致するものがあったらture
//     }
//     return false;                              // なかったらfalseを返す
// }

// // Vector配列に要素が重複しないように要素を追加する
// bool vec_union1(Vector *v, void *element){
//     if(vec_contains(v, element)) return false; // すでに値が存在していれば，何もせずfalseを返す
//     vec_push(v, element);                      // 存在しなければ，Vector配列の末尾に要素を追加
//     return true;                               // 正常終了のtrueを返す
// }

// // 新しいmap配列をつくる
// Map *new_map(void){
//     Map *map = malloc(sizeof(Map)); // メモリ割り当て
//     map->keys = new_vec();          // キーを格納するVector配列を作る 
//     map->vals = new_vec();          // 値を格納するVector配列を作る
//     return map;                     // 初期化されたmapのポインタを返す
// }

// // Mapの末尾に要素を追加
// void map_put(Map *map, char *key, void *val){
//     vec_push(map->keys, key);       // mapのキーの配列の末尾に新しいキーを追加
//     vec_push(map->vals, val);       // mapの値の配列の末尾に新しい値を追加
// }

// // Mapの末尾に整数型の要素を追加
// void map_puti(Map *map, char *key, int val){
//     map_put(map, key, (void *)(intptr_t) val); // intptr_tは整数型をポインタ型に変換
// }

// // 指定されたキーに対応する値をMapから取得し、その値を返す
// void *map_get(Map *map, char *key){
//     for(int i = map->keys->len - 1; i >= 0; i--){
//         if(!strcmp(map->keys->data[i], key)){  // 現在検索しているキーが指定されたキーと一致するかどうかを確認
//             return map->vals->data[i];         // キーが一致した場合，対応する値(vals)を返す
//         }
//     }
//     return NULL;                               // 存在しない場合，NULLを返す
// }

// // 指定されたキーに対応する整数値をMapから取得し，その値を返す
// int map_geti(Map *map, char *key, int default_){
//     for(int i = map->keys->len - 1; i >= 0; i--){
//         if(!strcmp(map->keys->data[i], key)){    // 現在検索しているキーが指定されたキーと一致するかどうかを確認
//             return (intptr_t)map->vals->data[i]; // キーが一致した場合，対応する値(vals)を返す
//         }
//     }
//     return default_;                             // 指定されたキーがMap内で見つからなかった場合、_defaultを返す
// }

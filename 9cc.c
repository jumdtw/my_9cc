#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

enum {
    TK_NUM = 256,  //整数トークン
    TK_EOF,        //入力の終わりを表すトークン
};

typedef struct {
    int ty;         //トークン型
    int val;        //tyがTK_NUMの場合、その数値
    char *input;    //トークン文字列
}Token;

//トークンない図した結果のトークン列はこの配列に保存する
//100個以上のトークンはこないものとする
Token tokens[100];

//pが指名しているボジ列をトークンに分割してtokensに保存する
void tokenize(char *p){
    int i = 0;
    while(*p){
        //空白文字をスキップ
        if(isspace(*p)){
            p++;
            continue;
        }
        //
        if(*p == '+' || *p == '-'){
            tokens[i].ty = *p;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }
        //10進数の数字であるか否か
        if(isdigit(*p)){
            tokens[i].ty = TK_NUM;
            tokens[i].input = p;
            tokens[i].val = strtol(p,&p,10);
            i++;
            continue;
        }

        fprintf(stderr,"トークンナイズできません: %s\n",p);
        exit(1);
    }
    tokens[i].ty = TK_EOF;
    tokens[i].input = p;
}

void error(int i){
    fprintf(stderr,"予期しないトークンです: %s\n",tokens[i].input);
    exit(1);
}

int main(int argc,char **argv){
    if(argc != 2){
        fprintf(stderr,"引数の個数が正しくありません\n");
        return 1;
    }
    
    //トークンナイズ
    tokenize(argv[1]);



    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
    if(tokens[0].ty != TK_NUM){
        error(0);
    }
    printf("    mov rax, %d\n",tokens[0].val);

    //+あるいは-というトークンの並びを消費しつつ
    //アセンブリ出力
    int i = 1;
    while(tokens[i].ty != TK_EOF){
        if(tokens[i].ty == '+'){
            i++;
            if(tokens[i].ty != TK_NUM){
                error(i);
            }
            printf("    add rax, %d\n",tokens[i].val);
            i++;
            continue;
        }

        if(tokens[i].ty == '-'){
            i++;
            if(tokens[i].ty != TK_NUM){
                error(i);
            }
            printf("    sub rax,%d\n", tokens[i].val);
            i++;
            continue;
        }

        
        error(i);
    }

    printf("    ret\n");
    return 0;
}
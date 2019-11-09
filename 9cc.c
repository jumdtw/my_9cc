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
    int len;
}Token;

enum{
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
};

typedef struct Node Node;

struct Node{
    int ty;
    Node *lhs;
    Node *rhs;
    int val;
};

Node *expr();

//トークンない図した結果のトークン列はこの配列に保存する
//100個以上のトークンはこないものとする
Token tokens[100];
int pos = 0;

Node *new_node(int ty,Node *lhs, Node *rhs){
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val){
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

int consume(int ty){
    if(tokens[pos].ty != ty){
        return 0;
    }
    pos++;
    return 1;
}

Node *primary(){
    if(consume('(')){
        Node *node = expr();
        if(consume(')')){
            return node;
        }
    }
    if(tokens[pos].ty == TK_NUM){
        return new_node_num(tokens[pos++].val);
    }
    
}

Node *unary(){
    if(consume(ND_ADD)){
        return primary();
    }
    if(consume(ND_SUB)){
        return new_node(ND_SUB,new_node_num(0),primary());
    }
    return primary();
}

Node *mul(){
    Node *node = unary();

    for(;;){
        if(consume(ND_MUL)){
            node = new_node(ND_MUL,node,unary());
        }else if(consume(ND_DIV)){
            node = new_node(ND_DIV,node,unary());
        }else{
            return node;
        }
    }
}

Node *expr(){
    Node *node = mul();

    for(;;){
        if(consume(ND_ADD)){
            node = new_node(ND_ADD,node,mul());
        }else if(consume(ND_SUB)){
            node = new_node(ND_SUB,node,mul());
        }else{
            return node;
        }
    }

}


void gen(Node *node){
    if(node->ty == ND_NUM){
        printf("    push %d\n",node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch(node->ty){
        case ND_ADD:
            printf("    add rax, rdi\n");
            break;
        case ND_SUB:
            printf("    sub rax ,rdi\n");
            break;
        case ND_MUL:
            printf("    mul rdi\n");
            break;
        case ND_DIV:
            printf("    mov rdx, 0\n");
            printf("    div rdi\n");
            break;
    }

    printf("    push rax\n");
}

//pが指名しているボジ列をトークンに分割してtokensに保存する
void tokenize(char *p){
    int i = 0;
    while(*p){
        //空白文字をスキップ
        if(isspace(*p)){
            p++;
            continue;
        }
        if(*p=='+'){
            tokens[i].ty = ND_ADD;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }
        if(*p=='-'){
            tokens[i].ty = ND_SUB;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }
        if(*p=='*'){
            tokens[i].ty = ND_MUL;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }
        if(*p=='/'){
            tokens[i].ty = ND_DIV;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }
        if(*p == '(' || *p == ')'){
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
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 抽象構文木を下りながらコード生成
    gen(node);

    // スタックトップに式全体の値が残っているはずなので
    // それをRAXにロードして関数からの返り値とする
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}
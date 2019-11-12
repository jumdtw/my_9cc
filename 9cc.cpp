#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<vector>



enum {
    TK_NUM = 256,  //整数トークン
    TK_EOF,        //入力の終わりを表すトークン
};


typedef struct {
    int ty;         //トークン型
    int val;        //tyがTK_NUMの場合、その数値
    char *str;    //トークン文字列
    int len;
}Token;

enum{
    ND_NUM,
    // 比較演算
    ND_SETE,  // ==
    ND_SETL,  // <
    ND_SETLE, // <=
    ND_SETNE, // !=
};
// 四則演算
#define ND_ADD "+"
#define ND_SUB "+"
#define ND_MUL "+"
#define ND_DIV "+"
#define ND_PARENR "("
#define ND_PARENL ")"


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
std::vector<Token> tokens;
int pos = 0;

Node *new_node(int ty,Node *lhs, Node *rhs){
    Node *node = (Node *)malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val){
    Node *node = (Node *)malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

bool consume(char *op){
    if(memcmp(tokens[pos].str,op,tokens[pos].len)){
        return false;
    }
    //printf("tokens.ty == : %d\n" ,tokens[pos].ty);
    pos++;
    return true;
}

Node *primary(){
    if(consume((char*)"(")){
        Node *node = expr();
        if(consume((char*)")")){
            return node;
        }
    }

    return new_node_num(tokens[pos++].val);
    
    
}

Node *unary(){
    if(consume((char*)"+")){
        return primary();
    }
    if(consume((char*)"-")){
        return new_node('-',new_node_num(0),primary());
    }
    return primary();
}

Node *mul(){
    Node *node = unary();

    for(;;){
        if(consume((char*)"*")){
            node = new_node('*',node,unary());
        }else if(consume((char*)"/")){
            node = new_node('/',node,unary());
        }else{
            return node;
        }
    }
}

Node *expr(){
    Node *node = mul();

    for(;;){
        if(consume((char*)"+")){
            node = new_node('+',node,mul());
        }else if(consume((char*)"-")){
            node = new_node('-',node,mul());
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
        case '+':
            printf("    add rax, rdi\n");
            break;
        case '-':
            printf("    sub rax ,rdi\n");
            break;
        case '*':
            printf("    mul rdi\n");
            break;
        case '/':
            printf("    mov rdx, 0\n");
            printf("    div rdi\n");
            break;
        case ND_SETE: // == 
            printf("    cmp rax, rdi\n");
            printf("    sete al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_SETL:  // <
            printf("    cmp rax, rdi\n");
            printf("    setl al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_SETLE: // <=
            printf("    cmp rax, rdi\n");
            printf("    setle al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_SETNE: // !=
            printf("    cmp rax, rdi\n");
            printf("    setne al\n");
            printf("    movzb rax, al\n");
            break;
    }

    printf("    push rax\n");
}

//pが指名しているボジ列をトークンに分割してtokensに保存する
void tokenize(char *p){
    int i = 0;
    Token end;
    char sete[] = "==",setne[] = "!=",setle[] = "<=";
    while(*p){
        Token token;
        //空白文字をスキップ
        if(isspace(*p)){
            p++;
            continue;
        }
        // arr1 == arr2 の時の戻り値が０
        // == 
        if(!memcmp(sete,p,2)){    
            token.ty = ND_SETE;
            token.str = p;
            token.len = 2;
            tokens.push_back(token);
            p++;
            continue;
        }
        // !=
        if(!memcmp(setne,p,2)){    
            token.ty = ND_SETNE;
            token.str = p;
            token.len = 2;
            tokens.push_back(token);
            p++;
            continue;
        }
        // <=
        if(!memcmp(setle,p,2)){    
            token.ty = ND_SETLE;
            token.str = p;
            token.len = 2;
            tokens.push_back(token);
            p++;
            continue;
        }
        
        if(*p == '(' || *p == ')' || *p=='+' || *p=='-' || *p=='*' || *p=='/' || *p=='<' || *p=='>'){    
            token.ty = *p;
            token.str = p;
            token.len = 1;
            tokens.push_back(token);
            p++;
            continue;
        }
        //10進数の数字であるか否か
        if(isdigit(*p)){
            token.ty = TK_NUM;
            token.str = p;
            token.val = strtol(p,&p,10);
            tokens.push_back(token);
            continue;
        }

        fprintf(stderr,"トークンナイズできません: %s\n",p);
        exit(1);
    }
    end.ty = TK_EOF;
    end.str = p;
    tokens.push_back(end);
    
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
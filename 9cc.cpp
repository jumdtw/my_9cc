#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include<vector>



enum {
    TK_NUM = 256,  //整数トークン
    TK_RETURN,
    TK_IF,
    TK_ELSE,
    TK_WHILE,
    TK_IDENT,     // 識別子
    TK_SETE,  // ==
    TK_SETL,  // <
    TK_SETLE, // <=
    TK_SETNE, // !=
    TK_EOF,        //入力の終わりを表すトークン
};

enum{
    ND_NUM,
    // 比較演算
    ND_ASSIGN,
    ND_RETURN,
    ND_IF,
    ND_ELSE,
    ND_WHILE,
    ND_IDENT,
    ND_LVAR,
    ND_BLOCK,
    ND_SETE,  // ==
    ND_SETL,  // <
    ND_SETLE, // <=
    ND_SETNE, // !=
};


typedef struct Node Node;
typedef struct LVar LVar;

struct Node{
    int ty;
    Node *lhs;
    Node *rhs;
    std::vector<Node*> stmts;
    int val;
    int offset;
    char *str;
};

typedef struct {
    int ty;         //トークン型
    int val;        //tyがTK_NUMの場合、その数値
    char *str;    //トークン文字列
    int len;
}Token;

struct LVar{
    LVar *next; 
    char *name; //変数名
    int len;    //name.len()
    int offset; //
};

Node *expr();

// tokenizeの結果がここに入る
std::vector<Token> tokens;
// 構文木がここにはいっている
std::vector<Node*> code;
LVar *locals = NULL;
int pos = 0;


LVar *find_lvar(Token *tok){
    for(LVar *var=locals;var;var=var->next){
        if(var->len==tok->len&&!memcmp(tok->str,var->name,var->len)){
            return var;
        }
    }
    return NULL;
}


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
    pos++;
    return true;
}

bool at_eof(){
    return tokens[pos].ty == TK_EOF;
}

void expect(char *p){
    if(!memcmp(tokens[pos].str,p,tokens[pos].len)){
        return;
    }
    printf("error expect %s is not %s\n",p,tokens[pos].str);
    exit(1);
}

Node *primary(){
    if(consume((char*)"(")){
        Node *node = expr();
        if(consume((char*)")")){
            return node;
        }
    }

    if(tokens[pos].ty == TK_NUM){
        return new_node_num(tokens[pos++].val);
    }
    
    if(tokens[pos].ty==TK_IDENT){

        Node *node = (Node*)malloc(sizeof(Node));
        node->ty = ND_LVAR;
        LVar *lvar = find_lvar(&tokens[pos]);

        if(lvar){
            node->offset = lvar->offset;
        }else{
            lvar = (LVar*)malloc(sizeof(LVar));
            lvar->next = locals;
            lvar->name = tokens[pos].str;
            lvar->len = tokens[pos].len;
            if(locals==NULL){
                lvar->offset = 8;
            }else{
                lvar->offset = locals->offset + 8;
            }
            node->offset = lvar->offset;
            locals = lvar;
        }
        pos++;
        return node;
    }
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

Node *add(){
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

Node *relational(){
    Node *node = add();
    char setle[] = "<=",setre[] = ">=";
    for(;;){
        if(consume((char*)"<")){
            node = new_node(ND_SETL,node,add());
        }else if(consume((char*)">")){
            node = new_node(ND_SETL,add(),node);
        }else if(consume(setle)){  // <=
            node = new_node(ND_SETLE,node,add());
        }else if(consume(setre)){  // >=
            node = new_node(ND_SETLE,add(),node);
        }else{
            return node;
        }
    }

}

Node *equality(){
    Node *node = relational();

    for(;;){
        if(tokens[pos].ty==TK_SETE){  // == 
            pos++;
            node = new_node(ND_SETE,node,relational());
        }else if(tokens[pos].ty==TK_SETNE){ // !=
            pos++;
            node = new_node(ND_SETNE,node,relational());
        }else{
            return node;
        }
    }
}


Node *assign(){
    Node *node = equality();
    if(consume((char*)"=")){
        node = new_node(ND_ASSIGN,node,assign());
    }
    return node;

}

Node *expr(){
    return assign();
}


Node *stmt(){
    Node *node;

    if(tokens[pos].ty==TK_RETURN){
        node = (Node*)malloc(sizeof(Node));
        node->ty = ND_RETURN;
        pos++;
        node->lhs = expr();
        expect((char*)";");
        pos++;
        return node;
    }
    
    if(tokens[pos].ty==TK_IF){
        pos++;
        if(consume((char*)"(")){
            node = (Node*)malloc(sizeof(Node));
            node->ty = ND_IF;
            node->lhs = expr();
            if(consume((char*)")")){
                Node *else_node = stmt();
                if(tokens[pos].ty==TK_ELSE){
                    pos++;
                    node->rhs = new_node(ND_ELSE,else_node,stmt());
                    return node;
                }
                node->rhs = else_node;
                return node; 
            }
        }
    }

    if(tokens[pos].ty==TK_WHILE){
        pos++;
        if(consume((char*)"(")){
            node = (Node*)malloc(sizeof(Node));
            node->ty = ND_WHILE;
            node->lhs = expr();
            if(consume((char*)")")){
                node->rhs = stmt();
                return node; 
            }
        }
    }
    
    if(consume((char*)"{")){
        node = (Node*)malloc(sizeof(Node));
        node->ty = ND_BLOCK;
        while(!consume((char*)"}")){
            node->stmts.push_back(stmt());
        }
        return node;
    }
    
    
    node = expr();
    expect((char*)";");
    pos++;
    return node;
}

void program(){
    Node *end = NULL;
    while (!at_eof()){
        code.push_back(stmt());
    }
    code.push_back(end);
}

void gen_lval(Node *node){
    if(!node->ty==ND_LVAR){
        printf("not ND_LVAR");
        exit(1);
    }
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n",node->offset);
    printf("    push rax\n");
}


void gen(Node *node){
    if(node->ty==ND_BLOCK){
        for(int i=0;i<node->stmts.size();i++){
            gen(node->stmts[i]);
        }
        return;
    }
    if(node->ty == ND_RETURN){
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    }
    if(node->ty == ND_IF){
        srand((unsigned int)time(NULL));
        int L = rand()%10000;
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        
        if(node->rhs->ty==ND_ELSE){
            printf("    je  .%delse\n",L);
            gen(node->rhs->lhs);
            printf("    jmp .%dend\n",L);
            printf(".%delse:\n",L);
            gen(node->rhs->rhs);
        }else{
            printf("    je  .%dend\n",L);
            gen(node->rhs);
        }
        printf(".%dend:\n",L);
        return;
    }

    if(node->ty == ND_WHILE){
        srand((unsigned int)time(NULL));
        int L = rand()%10000;
        printf(".%dbegin:\n",L);
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        printf("    je  .%dend\n",L);
        gen(node->rhs);
        printf("    jmp .%dbegin\n",L);
        printf(".%dend:\n",L);
        return;
    }

    if(node->ty == ND_NUM){
        printf("    push %d\n",node->val);
        return;
    }else if(node->ty==ND_LVAR){
        gen_lval(node);
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    }else if(node->ty==ND_ASSIGN){
        gen_lval(node->lhs);
        gen(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        printf("    push rdi\n");
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

int lvar_len(char *p){
    int len = 0;
    for(;;){
        if(('a' <= *p && *p <= 'z')||('A'<=*p && *p<='Z'||'0'<=*p && *p <= '9')){
            p++;
            len++;
        }else{
            return len;
        }
    }
}

int is_alnum(char c){
    return  ('a' <= c && c <= 'z') ||
            ('A' <= c && c <= 'Z') ||
            ('0' <= c && c <= '9') ||
            (c == '_');

}

//pが指名しているボジ列をトークンに分割してtokensに保存する
void tokenize(char *p){
    int i = 0;
    Token end;
    char sete[] = "==",setne[] = "!=",setle[] = "<=",setle_re[] = ">=";
    while(*p){
        Token token;
        //空白文字をスキップ
        if(isspace(*p)||*p=='\n'){
            p++;
            continue;
        }
        // arr1 == arr2 の時の戻り値が０
        // == 
        if(!memcmp(sete,p,2)){    
            token.ty = TK_SETE;
            token.str = p;
            token.len = 2;
            tokens.push_back(token);
            p+=2;
            continue;
        }
        // !=
        if(!memcmp(setne,p,2)){    
            token.ty = TK_SETNE;
            token.str = p;
            token.len = 2;
            tokens.push_back(token);
            p+=2;
            continue;
        }
        // <= or >=
        if(!memcmp(setle,p,2)||!memcmp(setle_re,p,2)){    
            token.ty = TK_SETLE;
            token.str = p;
            token.len = 2;
            tokens.push_back(token);
            p+=2;
            continue;
        }
        
        if(*p == '(' || *p == ')' || *p=='+' || *p=='-' || *p=='*' || *p=='/' || *p=='<' || *p=='>' || *p=='=' || *p == ';' || *p=='{' || *p=='}'){    
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

        // return文の判別
        if(strncmp(p,"return",6)==0&&!is_alnum(p[6])){
            token.ty = TK_RETURN;
            token.str = p;
            p+=6;
            tokens.push_back(token);
            continue;
        }

        //if文の判別
        if(strncmp(p,"if",2)==0&&!is_alnum(p[2])){
            token.ty = TK_IF;
            token.str = p;
            p+=2;
            tokens.push_back(token);
            continue;
        }
        //else文の判別
        if(strncmp(p,"else",4)==0&&!is_alnum(p[4])){
            token.ty = TK_ELSE;
            token.str = p;
            p+=4;
            tokens.push_back(token);
            continue;
        }
        //while文の判別
        if(strncmp(p,"while",5)==0&&!is_alnum(p[5])){
            token.ty = TK_WHILE;
            token.str = p;
            p+=5;
            tokens.push_back(token);
            continue;
        }
        
        //変数の判別
        if('a' <= *p && *p <= 'z'){
            token.ty = TK_IDENT;
            token.str = p;
            token.len = lvar_len(p);
            p += token.len;  // ここでたしてあげないと文字数分ずれない。
            tokens.push_back(token);
            continue;
        }


        fprintf(stderr,"トークンナイズできません: %s\n",p);
        exit(1);
    }
    end.ty = TK_EOF;
    tokens.push_back(end);
    
    return;
}


int main(int argc,char **argv){
    if(argc != 2){
        fprintf(stderr,"引数の個数が正しくありません\n");
        return 1;
    }
    
    //トークンナイズ
    tokenize(argv[1]);
    /*
    for(int i=0;i<tokens.size();i++){
        printf("ty -> %d\n",tokens[i].ty);
        printf("val -> %d\n",tokens[i].val);
        printf("str -> %s\n",tokens[i].str);
        printf("len -> %d\n",tokens[i].len);
    }
    */

    program();
    
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");

    // 抽象構文木を下りながらコード生成
    for(int i=0;code[i];i++){
        gen(code[i]);
    }

    return 0;
}
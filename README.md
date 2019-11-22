# 9cc

# ebnf

program = ident ( ) { stmt* }

stmt =  expr ";"
     |  "{" stmt* "}"
     |  "if" "(" expr ")" stmt ( "else" stmt )?
     |  "while" "(" expr ")" stmt
     |  "return" expr ";"

expr = assign

assign = equality ("=" assign)?

equality = relational ("==" relational | "!=" relational)*

relational = add ("<" add | "<=" add | ">" add | ">=" add)*

add = mul ("+" mul | "-" mul)*

mul = unary ("*" unary | "/" unary)*

unary = num | ident ( "(" ")" )? | "(" expr ")"

# 問題点
毎回スタックを208減算しているのでいつかしぬ
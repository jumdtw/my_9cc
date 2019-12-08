# 9cc

# ebnf

program = ("int" | "double" | ...) ident ( ) { stmt* }

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

unary =  "+"? primary | "-"? primary | "*"? unary | "&"? unary

primary = num | ("int" | "double" | ...)? ident ( "(" ")" )? | "(" expr ")"

# call  関数処理
以下のような記述がきたときの処理
foo(1,2);

引数なしの呼び出しに関しては簡単なので割愛

めんどくさいのは引数がある場合。設計上変数はoffsetしか記憶していないので

mov rax offset
push rax

のようにoffsetの値を一度raxにうつしてからpushする。メモリの無駄遣いであるがしかたない

# 関数定義
以下のような記述がきたときの処理
foo(a,b){
    stmt*
}

やってること自体はprogramでidentを識別してfunc構造体にcodeをぶち込んでるだけ。

引数はあらかじめ変数リストにいれておけばいい。

それっぽいもの書いたけどまちがっている

rbp **+** 8*nの場所がオフセットでなきゃだめ



# 問題点
ビルドインを呼ぶ時に引数をつけられない。

自作関数に引数をつけることができない

# 6502に移植

前提としてスタックに計算したい値２つがあらかじめ積んである。
また、x86との違いとそれの対処方も記す

## レジスタの違いの対処方

- 算術用レジスタが一つしかない
  $0 メモリを算術用のメモリとして使用する

- rbpが存在しない。
  $1 をrbpとして代用する。



## add

clc    //未定義 キャリーフラグをクリア
pla  //未定義 aにポップ
sta $0
pla  //未定義 aにポップ
adc $0//未定義
pha   //未定義 aをプッシュ

## sub

clc    //未定義 キャリーフラグをクリア
pla  //未定義 aにポップ
sta $0
pla  //未定義 aにポップ
sbc $0//未定義
pha   //未定義 aをプッシュ



# test code


.delay  dec a
        